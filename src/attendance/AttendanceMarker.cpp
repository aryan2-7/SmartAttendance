#include "AttendanceMarker.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

AttendanceMarker::AttendanceMarker() = default;

// ── Initialize ────────────────────────────────────────────────────────────
bool AttendanceMarker::initialize(const std::string &detectorModel,
                                  const std::string &recognizerModel,
                                  const std::string &galleryDir) {
    detector_ = cv::FaceDetectorYN::create(
        detectorModel, "", cv::Size(640, 480), 0.9f, 0.3f, 5000);
    if (!detector_) { std::cerr << "YuNet load failed\n"; return false; }

    recognizer_ = cv::FaceRecognizerSF::create(recognizerModel, "");
    if (!recognizer_) { std::cerr << "SFace load failed\n"; return false; }

    if (!loadGallery(galleryDir)) { std::cerr << "Gallery empty\n"; return false; }

    cap_.open(0);
    if (!cap_.isOpened()) { std::cerr << "Webcam open failed\n"; return false; }

    return true;
}

// ── Load .bin gallery files ───────────────────────────────────────────────
bool AttendanceMarker::loadGallery(const std::string &galleryDir) {
    for (auto &entry : fs::directory_iterator(galleryDir)) {
        if (entry.path().extension() != ".bin") continue;

        // Filename format: "Aryan_Khatri_43.bin"
        std::string stem = entry.path().stem().string(); // "Aryan_Khatri_43"
        size_t lastUs = stem.rfind('_');
        if (lastUs == std::string::npos) continue;
        std::string roll     = stem.substr(lastUs + 1);
        std::string namePart = stem.substr(0, lastUs);
        std::replace(namePart.begin(), namePart.end(), '_', ' ');

        // Read binary gallery mat
        std::ifstream ifs(entry.path().string(), std::ios::binary);
        int rows = 0, cols = 0;
        ifs.read(reinterpret_cast<char*>(&rows), sizeof(int));
        ifs.read(reinterpret_cast<char*>(&cols), sizeof(int));
        cv::Mat gallery(rows, cols, CV_32F);
        ifs.read(reinterpret_cast<char*>(gallery.data),
                 static_cast<std::streamsize>(rows * cols * sizeof(float)));

        students_.push_back({namePart, roll, gallery});
        std::cout << "Loaded: " << namePart << " (Roll " << roll << ")\n";
    }
    return !students_.empty();
}

// ── Detect best face ──────────────────────────────────────────────────────
bool AttendanceMarker::detectBestFace(const cv::Mat &frame, cv::Mat &faceBox) {
    detector_->setInputSize(frame.size());
    cv::Mat faces;
    detector_->detect(frame, faces);
    if (faces.rows < 1) return false;

    int best = 0;
    for (int i = 1; i < faces.rows; ++i)
        if (faces.at<float>(i, 14) > faces.at<float>(best, 14)) best = i;

    faceBox = faces.row(best);
    return true;
}

// ── Match feature vector against all enrolled students ────────────────────
// Returns index into students_ with highest cosine similarity above threshold.
// Returns -1 if no match.
int AttendanceMarker::matchFace(const cv::Mat &feat) {
    int    bestIdx   = -1;
    double bestScore = COSINE_THRESHOLD;

    for (int s = 0; s < static_cast<int>(students_.size()); ++s) {
        const cv::Mat &gallery = students_[s].gallery;
        // Compare query feat against every enrolled sample; take max score
        for (int r = 0; r < gallery.rows; ++r) {
            cv::Mat sample = gallery.row(r);
            double score = recognizer_->match(feat, sample,
                                              cv::FaceRecognizerSF::FR_COSINE);
            if (score > bestScore) {
                bestScore = score;
                bestIdx   = s;
            }
        }
    }
    return bestIdx;
}

// ── Liveness check (blink state machine) ─────────────────────────────────
bool AttendanceMarker::livenessCheck(int idx, const cv::Mat &frame,
                                     const cv::Mat &faceBox) {
    if (eyeCascade_->empty()) return true; // degrade gracefully if no cascade

    int x = static_cast<int>(faceBox.at<float>(0, 0));
    int y = static_cast<int>(faceBox.at<float>(0, 1));
    int w = static_cast<int>(faceBox.at<float>(0, 2));
    int h = static_cast<int>(faceBox.at<float>(0, 3));

    cv::Rect faceRect(x, y, w, h);
    faceRect &= cv::Rect(0, 0, frame.cols, frame.rows); // clamp to image
    cv::Mat faceROI = frame(faceRect);
    cv::Mat upperHalf = faceROI(cv::Rect(0, 0, faceROI.cols, faceROI.rows / 2));
    cv::Mat gray;
    cv::cvtColor(upperHalf, gray, cv::COLOR_BGR2GRAY);

    std::vector<cv::Rect> eyes;
    eyeCascade_->detectMultiScale(gray, eyes, 1.1, 3, 0, {20, 20});

    int &state = blinkState_[idx];
    switch (state) {
        case 0: if (eyes.size() >= 2) state = 1; break;
        case 1: if (eyes.empty())     state = 2; break;
        case 2: if (!eyes.empty())    state = 3; break;
        case 3: return true;
    }
    return false;
}

// ── Log attendance to CSV ─────────────────────────────────────────────────
void AttendanceMarker::logAttendance(const StudentRecord &s, double score,
                                     const std::string &status) {
    auto &last = lastLogged_[s.roll];
    auto now   = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - last).count()
            < COOLDOWN_SECONDS) return;
    last = now;

    // Wall-clock timestamp
    auto t  = std::time(nullptr);
    auto tm = *std::localtime(&t);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);

    std::ofstream csv("attendance.csv", std::ios::app);
    // Write header if file is empty
    if (csv.tellp() == 0)
        csv << "Name,Roll No,Score,Status,Timestamp\n";
    csv << s.name << "," << s.roll << ","
        << std::fixed << std::setprecision(4) << score << ","
        << status << "," << buf << "\n";
}

// ── Main recognition loop ─────────────────────────────────────────────────
void AttendanceMarker::run() {
    cv::Mat frame;
    int autoClose = 0;

    while (true) {
        cap_ >> frame;
        if (frame.empty()) break;
        cv::flip(frame, frame, 1);

        cv::Mat faceBox;
        if (detectBestFace(frame, faceBox)) {
            // Align and extract feature
            cv::Mat aligned, feat;
            recognizer_->alignCrop(frame, faceBox, aligned);
            recognizer_->feature(aligned, feat);

            int idx = matchFace(feat);

            // Draw bounding box
            int x = static_cast<int>(faceBox.at<float>(0, 0));
            int y = static_cast<int>(faceBox.at<float>(0, 1));
            int w = static_cast<int>(faceBox.at<float>(0, 2));
            int h = static_cast<int>(faceBox.at<float>(0, 3));

            if (idx == -1) {
                cv::rectangle(frame, {x, y}, {x+w, y+h}, {0, 0, 255}, 2);
                cv::putText(frame, "Unknown", {x, y - 10},
                            cv::FONT_HERSHEY_SIMPLEX, 0.8, {0, 0, 255}, 2);
                autoClose = 0;
            } else {
                const StudentRecord &s = students_[idx];

                // Re-score: cosine similarity vs mean gallery vector (for display)
                double score = 0;
                for (int r = 0; r < s.gallery.rows; ++r)
                    score = std::max(score,
                        recognizer_->match(feat, s.gallery.row(r),
                                           cv::FaceRecognizerSF::FR_COSINE));

                bool blinked = livenessCheck(idx, frame, faceBox);
                std::string label;
                cv::Scalar  color;

                if (!blinked) {
                    label = s.name + " - BLINK PLEASE";
                    color = {0, 255, 255};
                    autoClose = 0;
                } else {
                    label = s.name + " - PRESENT";
                    color = {0, 255, 0};
                    logAttendance(s, score, "Present");
                    autoClose++;
                }

                cv::rectangle(frame, {x, y}, {x+w, y+h}, color, 2);
                cv::putText(frame, label, {x, y - 10},
                            cv::FONT_HERSHEY_SIMPLEX, 0.7, color, 2);
            }
        } else {
            autoClose = 0;
        }

        cv::imshow("SmartAttendance - Scanner", frame);
        if (cv::waitKey(30) == 'q') break;
        if (autoClose > 45) break; // ~1.5 s after confirmed present
    }

    cap_.release();
    cv::destroyAllWindows();
}