#include "AttendanceMarker.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <filesystem>
#include <algorithm>

AttendanceMarker::AttendanceMarker() : closeCounter_(0) {
    recognizer_ = cv::face::LBPHFaceRecognizer::create(LBPH_RADIUS, LBPH_NEIGHBORS, LBPH_GRID_X, LBPH_GRID_Y);
}

AttendanceMarker::~AttendanceMarker() {
    if (cap_.isOpened()) {
        cap_.release();
    }
    cv::destroyAllWindows();
}

bool AttendanceMarker::initialize(const std::string& cascadePath, const std::string& eyeCascadePath, const std::string& modelPath) {
    if (!faceCascade_.load(cascadePath)) {
        std::cerr << "[Error] Could not load Haar Cascade XML from " << cascadePath << "\n";
        return false;
    }

    if (!eyeCascade_.load(eyeCascadePath)) {
        std::cerr << "[Error] Could not load Eye Haar Cascade XML from " << eyeCascadePath << "\n";
        return false;
    }

    try {
        recognizer_->read(modelPath);
        std::cout << "[Success] Loaded LBPH face recognition model from " << modelPath << "\n";
    } catch (const cv::Exception& e) {
        std::cerr << "[Error] Failed to load .yml model. Did Aryan generate it yet?\n";
        std::cerr << "OpenCV Error: " << e.what() << "\n";
        return false;
    }

    if (!preprocessor_.initialize("resources/models/lbfmodel.yaml")) {
        std::cerr << "[Warning] LBF facemark model failed to load — proceeding without alignment.\n";
    }

    cap_.open(0);
    if (!cap_.isOpened()) {
        std::cerr << "[Error] Failed to open webcam.\n";
        return false;
    }

    cv::Mat warmup;
    for (int i = 0; i < 10; ++i) {
        cap_ >> warmup;
        cv::waitKey(100);
    }

    return true;
}

void AttendanceMarker::setLabelMap(const std::unordered_map<int, std::pair<std::string, std::string>>& map) {
    labelMap_ = map;
}

void AttendanceMarker::setThresholds(const std::unordered_map<int, double>& thresholds) {
    thresholds_ = thresholds;
}

std::string AttendanceMarker::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void AttendanceMarker::logAttendance(int labelId, double accuracy, const std::string& status) {
    auto now = std::chrono::steady_clock::now();
    
    if (lastLoggedTime_.find(labelId) != lastLoggedTime_.end()) {
        auto timeSinceLastLog = std::chrono::duration_cast<std::chrono::seconds>(now - lastLoggedTime_[labelId]).count();
        if (timeSinceLastLog < 300) {
            return;
        }
    }

    lastLoggedTime_[labelId] = now;

    std::string name = "Unknown";
    std::string rollNo = "Unknown";
    if (labelMap_.find(labelId) != labelMap_.end()) {
        name = labelMap_[labelId].first;
        rollNo = labelMap_[labelId].second;
    }

    std::ofstream file("attendance.csv", std::ios::app);
    if (file.is_open()) {
        std::string timestamp = getCurrentTimestamp();
        file << name << "," << rollNo << ",Accuracy: " << std::fixed << std::setprecision(2) << accuracy << "%," << status << "," << timestamp << "\n";
        file.close();
        
        std::string absPath = std::filesystem::absolute("attendance.csv").string();
        std::cout << ">>> ATTENDANCE RECORDED in: " << absPath << "\n";
        std::cout << "    Student: " << name << " (Roll No: " << rollNo << ") at " << timestamp << " (Status: " << status << ", Accuracy: " << std::fixed << std::setprecision(2) << accuracy << "%)\n";
    } else {
        std::cerr << "[Error] Could not write to attendance.csv\n";
    }
}

int AttendanceMarker::temporalVote() {
    if (recentPredictions_.empty()) return -1;

    std::unordered_map<int, int> counts;
    for (int label : recentPredictions_) {
        if (label != -1) counts[label]++;
    }

    int bestLabel = -1;
    int bestCount = 0;
    for (const auto& [label, count] : counts) {
        if (count > bestCount) {
            bestCount = count;
            bestLabel = label;
        }
    }

    int majorityThreshold = static_cast<int>(recentPredictions_.size()) / 2;
    if (bestCount > majorityThreshold) return bestLabel;
    return -1;
}

void AttendanceMarker::run() {
    std::cout << "\n=================================================\n";
    std::cout << "  SmartAttendance Scanner Active. Press 'q' to exit. \n";
    std::cout << "=================================================\n\n";

    std::ifstream checkFile("attendance.csv");
    if (!checkFile.good() || checkFile.peek() == std::ifstream::traits_type::eof()) {
        std::ofstream file("attendance.csv");
        file << "Name,Roll No,Accuracy,Status,Timestamp\n";
        file.close();
    }
    checkFile.close();

    recentPredictions_.clear();
    blinkState_.clear();
    blinkFrameCount_.clear();
    lastLoggedTime_.clear();
    loggedLabels_.clear();

    const std::string windowName = "SmartAttendance - Scanner";

    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
    {
        cv::Mat introFrame = cv::Mat::zeros(480, 640, CV_8UC3);
        cv::putText(introFrame, "Scanner starting...", cv::Point(150, 240),
                    cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
        cv::imshow(windowName, introFrame);
        cv::waitKey(1);
    }

    cv::Mat frame, grayFrame;

    while (true) {
        cap_ >> frame;
        if (frame.empty()) {
            std::cerr << "[Warning] Empty frame from camera, retrying...\n";
            cv::waitKey(100);
            continue;
        }

        cv::flip(frame, frame, 1);

        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

        // Apply CLAHE for detection — consistent with how FaceRegistration detects faces
        cv::Ptr<cv::CLAHE> detClahe = cv::createCLAHE(2.0, cv::Size(8, 8));
        cv::Mat enhancedGray;
        detClahe->apply(grayFrame, enhancedGray);

        std::vector<cv::Rect> faces;
        faceCascade_.detectMultiScale(enhancedGray, faces, HAAR_SCALE_FACTOR, HAAR_MIN_NEIGHBORS, 0, cv::Size(100, 100));

        for (const auto& face : faces) {
            cv::Mat processed;
            try {
                cv::Mat alignedFace = preprocessor_.alignFace(grayFrame, face);
                processed = preprocessor_.preprocess(alignedFace);
            } catch (const cv::Exception& e) {
                std::cerr << "[Warning] Preprocessor error: " << e.what() << "\n";
                cv::resize(grayFrame(face), processed, FacePreprocessor::kFaceCropSize);
            }

            int predictedLabel = -1;
            double confidence = 0.0;

            recognizer_->predict(processed, predictedLabel, confidence);

            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);

            double threshold = DEFAULT_THRESHOLD;
            if (thresholds_.find(predictedLabel) != thresholds_.end()) {
                threshold = thresholds_[predictedLabel];
            }

            double accuracy = 100.0 * (1.0 - (confidence / threshold));
            accuracy = std::max(0.0, std::min(100.0, accuracy));

            std::cout << "  [debug] label=" << predictedLabel
                      << " conf=" << std::fixed << std::setprecision(1) << confidence
                      << " acc=" << accuracy << "%"
                      << " thresh=" << threshold << "\n";

            if (predictedLabel == -1 || confidence > threshold) {
                cv::putText(frame, "Face Not Recognised", cv::Point(face.x, face.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
                recentPredictions_.push_back(-1);
                if (static_cast<int>(recentPredictions_.size()) > kTemporalWindow) {
                    recentPredictions_.pop_front();
                }
                continue;
            }

            recentPredictions_.push_back(predictedLabel);
            if (static_cast<int>(recentPredictions_.size()) > kTemporalWindow) {
                recentPredictions_.pop_front();
            }

            int stableLabel = temporalVote();

            std::string name = "Unknown";
            if (labelMap_.find(predictedLabel) != labelMap_.end()) {
                name = labelMap_[predictedLabel].first;
            }

            if (stableLabel == -1) {
                std::stringstream uiText;
                uiText << name << " - STABILISING (" << std::fixed << std::setprecision(1) << accuracy << "%)";
                cv::putText(frame, uiText.str(), cv::Point(face.x, face.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);
                closeCounter_ = 0;
                continue;
            }

            // ── Blink liveness test ──────────────────────────────────────────
            // State machine with consecutive-frame gating to filter cascade noise:
            //   0 → 1 : eyes open for ≥ OPEN_FRAMES consecutive frames
            //   1 → 2 : no eyes  for ≥ CLOSED_FRAMES consecutive frames  (the blink)
            //   2 → 3 : eyes open again for ≥ OPEN_FRAMES consecutive frames (blink complete)
            constexpr int OPEN_FRAMES   = 3;   // ~100 ms at 30 fps
            constexpr int CLOSED_FRAMES = 2;   // ~67  ms — a real blink is >100 ms

            cv::Mat faceROIForEyes = grayFrame(face);
            std::vector<cv::Rect> eyes;
            cv::Mat upperFace = faceROIForEyes(cv::Rect(0, 0, face.width, face.height / 2));
            eyeCascade_.detectMultiScale(upperFace, eyes, 1.1, 3, 0, cv::Size(20, 20));

            bool eyesOpen = (eyes.size() >= 2);
            int  state    = blinkState_[stableLabel];
            int  &fc      = blinkFrameCount_[stableLabel];   // consecutive-frame counter

            switch (state) {
                case 0:  // waiting for eyes to be clearly open
                    if (eyesOpen) {
                        fc++;
                        if (fc >= OPEN_FRAMES) { blinkState_[stableLabel] = 1; fc = 0; }
                    } else {
                        fc = 0;   // restart count
                    }
                    break;

                case 1:  // eyes were open — now waiting for them to close
                    if (!eyesOpen) {
                        fc++;
                        if (fc >= CLOSED_FRAMES) { blinkState_[stableLabel] = 2; fc = 0; }
                    } else {
                        fc = 0;
                    }
                    break;

                case 2:  // eyes were closed — waiting for them to reopen
                    if (eyesOpen) {
                        fc++;
                        if (fc >= OPEN_FRAMES) { blinkState_[stableLabel] = 3; fc = 0; }
                    } else {
                        fc = 0;
                    }
                    break;

                default:  // state 3 — blink already completed
                    break;
            }

            // ── UI + attendance logging ────────────────────────────────────────
            if (blinkState_[stableLabel] < 3) {
                // Blink not yet complete — show instruction, do NOT mark attendance
                std::stringstream uiText;
                uiText << name << " - PLEASE BLINK (" << std::fixed << std::setprecision(1) << accuracy << "%)";
                cv::putText(frame, uiText.str(), cv::Point(face.x, face.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);
                closeCounter_ = 0;
            } else {
                // Blink verified — mark PRESENT (only once per person per session)
                std::stringstream uiText;
                uiText << name << " - PRESENT (" << std::fixed << std::setprecision(1) << accuracy << "%)";
                cv::putText(frame, uiText.str(), cv::Point(face.x, face.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);

                if (loggedLabels_.find(stableLabel) == loggedLabels_.end()) {
                    logAttendance(stableLabel, accuracy, "Present");
                    loggedLabels_.insert(stableLabel);
                }

                closeCounter_++;
                if (closeCounter_ > 45) {
                    return;
                }
            }
        }

        cv::imshow(windowName, frame);

        if (cv::waitKey(30) == 'q' || cv::getWindowProperty(windowName, cv::WND_PROP_VISIBLE) < 1) {
            break;
        }
    }
}
