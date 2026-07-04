#include "AttendanceMarker.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <filesystem>
#include <algorithm>    // std::replace
#include <limits>       // std::numeric_limits

namespace fs = std::filesystem;

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor / Destructor
// ─────────────────────────────────────────────────────────────────────────────

AttendanceMarker::AttendanceMarker() {
    // models_ is empty at this point – filled in initialize()
}

AttendanceMarker::~AttendanceMarker() {
    if (cap_.isOpened()) cap_.release();
    cv::destroyAllWindows();
}


// ─────────────────────────────────────────────────────────────────────────────
//  initialize()
//
//  Loads ALL .yml files from `modelsDir` automatically – no manual selection.
//  Each file becomes one PersonModel with its own LBPHFaceRecognizer.
//  Name and roll number are parsed from the filename convention:
//      <FirstName>_<LastName>_<Roll>.yml
//  The last underscore-separated token is taken as the roll number.
// ─────────────────────────────────────────────────────────────────────────────

bool AttendanceMarker::initialize(const std::string& cascadePath,
                                   const std::string& eyeCascadePath,
                                   const std::string& modelsDir) {
    // ── Load cascade classifiers ──────────────────────────────────────────────
    if (!faceCascade_.load(cascadePath)) {
        std::cerr << "[Error] Could not load face Haar Cascade from: " << cascadePath << "\n";
        return false;
    }
    if (!eyeCascade_.load(eyeCascadePath)) {
        std::cerr << "[Error] Could not load eye Haar Cascade from: " << eyeCascadePath << "\n";
        return false;
    }

    // ── Auto-discover .yml models ─────────────────────────────────────────────
    if (!fs::exists(modelsDir)) {
        std::cerr << "[Error] Models directory not found: " << modelsDir << "\n";
        return false;
    }

    models_.clear();

    for (const auto& entry : fs::directory_iterator(modelsDir)) {
        if (entry.path().extension() != ".yml") continue;

        // Parse name and roll from filename stem (e.g. "Shashwot_Karki_36")
        std::string stem = entry.path().stem().string();
        PersonModel pm;

        size_t lastUnder = stem.rfind('_');
        if (lastUnder != std::string::npos) {
            pm.rollNo = stem.substr(lastUnder + 1);
            std::string namePart = stem.substr(0, lastUnder);
            std::replace(namePart.begin(), namePart.end(), '_', ' ');
            pm.name = namePart;
        } else {
            pm.name   = stem;
            pm.rollNo = "Unknown";
        }

        try {
            pm.recognizer = cv::face::LBPHFaceRecognizer::create();
            pm.recognizer->read(entry.path().string());
            models_.push_back(std::move(pm));
            std::cout << "[Loaded] " << models_.back().name
                      << " (Roll: " << models_.back().rollNo << ") from "
                      << entry.path().filename().string() << "\n";
        } catch (const cv::Exception& e) {
            std::cerr << "[Warning] Skipping " << entry.path().filename()
                      << " – " << e.what() << "\n";
        }
    }

    if (models_.empty()) {
        std::cerr << "[Error] No valid .yml models found in: " << modelsDir << "\n";
        return false;
    }

    std::cout << "[Success] Loaded " << models_.size() << " face model(s). Ready to scan.\n";

    // ── Open webcam ───────────────────────────────────────────────────────────
    cap_.open(0);
    if (!cap_.isOpened()) {
        std::cerr << "[Error] Failed to open webcam.\n";
        return false;
    }

    return true;
}


// ─────────────────────────────────────────────────────────────────────────────
//  predictBestMatch()
//
//  Runs faceROI through every loaded recognizer and returns the index of the
//  one with the LOWEST LBPH distance (= best match).
//  outConfidence receives that lowest distance.
// ─────────────────────────────────────────────────────────────────────────────

int AttendanceMarker::predictBestMatch(const cv::Mat& faceROI, double& outConfidence) const {
    if (models_.empty()) return -1;

    int    bestIdx  = 0;
    double bestConf = std::numeric_limits<double>::max();

    for (int i = 0; i < static_cast<int>(models_.size()); ++i) {
        int    lbl  = -1;
        double conf = 0.0;
        models_[i].recognizer->predict(faceROI, lbl, conf);
        if (conf < bestConf) {
            bestConf = conf;
            bestIdx  = i;
        }
    }

    outConfidence = bestConf;
    return bestIdx;
}


// ─────────────────────────────────────────────────────────────────────────────
//  getCurrentTimestamp()
// ─────────────────────────────────────────────────────────────────────────────

std::string AttendanceMarker::getCurrentTimestamp() {
    auto now   = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}


// ─────────────────────────────────────────────────────────────────────────────
//  logAttendance()
//
//  Writes one row to attendance.csv.
//  Has a 5-minute cooldown per person so the same person is never double-logged
//  in the same session.
// ─────────────────────────────────────────────────────────────────────────────

void AttendanceMarker::logAttendance(int personIdx, double accuracy, const std::string& status) {
    auto now = std::chrono::steady_clock::now();

    // ── 5-minute cooldown ─────────────────────────────────────────────────────
    if (lastLoggedTime_.count(personIdx)) {
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(
                        now - lastLoggedTime_.at(personIdx)).count();
        if (secs < 300) return; // still in cooldown – silently skip
    }
    lastLoggedTime_[personIdx] = now;

    const std::string& name   = models_[personIdx].name;
    const std::string& rollNo = models_[personIdx].rollNo;

    std::ofstream file("attendance.csv", std::ios::app);
    if (file.is_open()) {
        std::string ts = getCurrentTimestamp();
        file << name << "," << rollNo
             << ",Accuracy: " << std::fixed << std::setprecision(2) << accuracy << "%"
             << "," << status
             << "," << ts << "\n";
        file.close();

        std::string absPath = fs::absolute("attendance.csv").string();
        std::cout << ">>> ATTENDANCE RECORDED in: " << absPath << "\n"
                  << "    Student : " << name << " (Roll: " << rollNo << ")\n"
                  << "    Status  : " << status
                  << "  |  Accuracy: " << std::fixed << std::setprecision(1) << accuracy << "%"
                  << "  |  Time: " << ts << "\n\n";
    } else {
        std::cerr << "[Error] Could not write to attendance.csv\n";
    }
}


// ─────────────────────────────────────────────────────────────────────────────
//  run()
//
//  Main camera loop. Key behaviours:
//
//  1. AUTO-LOAD:  All models are already loaded – no manual selection.
//
//  2. MULTI-FACE: Every face in the frame is processed independently each tick.
//
//  3. STABILISATION (3-second window):
//     - When a face is first detected at >=30% accuracy, a 3-second timer starts.
//     - During those 3 seconds the student must blink (liveness check).
//     - The on-screen countdown shows remaining time so they know to stay still.
//     - After the 3-second window closes:
//         * Blink detected  → PRESENT  (logged)
//         * No blink        → ABSENT   (logged)
//     - If the person leaves the frame for >30 frames (~1 s) the timer resets,
//       giving them a fresh 3-second window when they return.
//
//  4. THRESHOLDS (unchanged):
//     - confidence > 130  → Unknown face, not logged.
//     - accuracy < 30%    → too low, stabilisation does NOT start.
//     - accuracy >= 30%   → eligible; blink test applies.
// ─────────────────────────────────────────────────────────────────────────────

void AttendanceMarker::run() {
    std::cout << "\n=================================================\n"
              << "  SmartAttendance Scanner Active.\n"
              << "  " << models_.size() << " model(s) loaded. Press 'q' to exit.\n"
              << "=================================================\n\n";

    // ── Ensure CSV header exists ───────────────────────────────────────────────
    {
        std::ifstream check("attendance.csv");
        if (!check.good() || check.peek() == std::ifstream::traits_type::eof()) {
            std::ofstream hdr("attendance.csv");
            hdr << "Name,Roll No,Accuracy,Status,Timestamp\n";
        }
    }

    cv::Mat frame, grayFrame;

    while (true) {
        auto now = std::chrono::steady_clock::now();

        cap_ >> frame;
        if (frame.empty()) break;

        // Mirror the frame to match registration orientation
        cv::flip(frame, frame, 1);

        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(grayFrame, grayFrame);

        // ── Detect faces ──────────────────────────────────────────────────────
        std::vector<cv::Rect> faces;
        faceCascade_.detectMultiScale(grayFrame, faces, 1.1, 4, 0, cv::Size(100, 100));

        // Track which person-indices were seen this frame so we can increment
        // the not-seen counter for everyone else.
        std::set<int> seenThisFrame;

        // ── Process each detected face ────────────────────────────────────────
        for (const auto& face : faces) {

            // Resize ROI to 200x200 – must match what FaceRegistration saves
            cv::Mat faceROI;
            cv::resize(grayFrame(face), faceROI, cv::Size(200, 200));

            double confidence = 0.0;
            int personIdx = predictBestMatch(faceROI, confidence);
            if (personIdx < 0) continue;

            // ── Convert LBPH distance → % accuracy ───────────────────────────
            // LBPH distance: 0 = perfect match, ~130+ = no match.
            // We cap the scale at 130 so that 0 → 100% and 130 → 0%.
            double accuracy = 100.0 * (1.0 - (confidence / 130.0));
            accuracy = std::max(0.0, std::min(100.0, accuracy));

            const std::string& name = models_[personIdx].name;

            // ── Unknown face (confidence too high) ────────────────────────────
            if (confidence > 130.0) {
                cv::rectangle(frame, face, cv::Scalar(0, 0, 255), 2);
                cv::putText(frame, "Unknown", cv::Point(face.x, face.y - 10),
                            cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
                continue;
            }

            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
            seenThisFrame.insert(personIdx);
            framesNotSeen_[personIdx] = 0;

            // ── Already marked recently? Show status and skip ─────────────────
            if (lastLoggedTime_.count(personIdx)) {
                auto secs = std::chrono::duration_cast<std::chrono::seconds>(
                                now - lastLoggedTime_.at(personIdx)).count();
                if (secs < 300) {
                    std::stringstream ss;
                    ss << name << " - ALREADY MARKED ("
                       << std::fixed << std::setprecision(1) << accuracy << "%)";
                    cv::putText(frame, ss.str(), cv::Point(face.x, face.y - 10),
                                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 200, 0), 2);
                    continue;
                }
            }

            // ── Accuracy too low – don't start timer ──────────────────────────
            if (accuracy < 30.0) {
                // Reset stabilisation if it was active – accuracy dropped back down
                stabilizationActive_[personIdx] = false;

                std::stringstream ss;
                ss << name << " - LOW ACCURACY ("
                   << std::fixed << std::setprecision(1) << accuracy << "%)";
                cv::putText(frame, ss.str(), cv::Point(face.x, face.y - 10),
                            cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 100, 255), 2);
                continue;
            }

            // ── Accuracy >= 30%: start or continue stabilisation window ────────
            bool isNewSession = !stabilizationActive_.count(personIdx)
                             || !stabilizationActive_.at(personIdx);
            if (isNewSession) {
                stabilizationStart_[personIdx]  = now;
                stabilizationActive_[personIdx] = true;
                blinkState_[personIdx]          = 0; // fresh blink-state for this window
                std::cout << "[Timer] Started 3-second window for: " << name << "\n";
            }

            // Compute elapsed time inside this stabilisation window
            double elapsedSecs = std::chrono::duration_cast<std::chrono::milliseconds>(
                                     now - stabilizationStart_.at(personIdx)).count() / 1000.0;
            double remainingSecs = std::max(0.0, 3.0 - elapsedSecs);

            // ── Blink detection (upper half of face ROI) ──────────────────────
            // State machine: 0→open(1)→closed(2)→open again(3=blinked)
            {
                cv::Mat upperFace = grayFrame(face)(
                    cv::Rect(0, 0, face.width, face.height / 2));
                std::vector<cv::Rect> eyes;
                eyeCascade_.detectMultiScale(upperFace, eyes, 1.1, 3, 0, cv::Size(20, 20));

                int& bState = blinkState_[personIdx];
                if      (bState == 0 && eyes.size() >= 2) bState = 1; // eyes open
                else if (bState == 1 && eyes.size() == 0) bState = 2; // eyes closed (blink)
                else if (bState == 2 && eyes.size() >= 1) bState = 3; // eyes reopened → blinked!
            }

            bool blinked = (blinkState_[personIdx] >= 3);

            // ── Still inside 3-second window ──────────────────────────────────
            if (elapsedSecs < 3.0) {
                std::stringstream ss;
                if (!blinked) {
                    // Prompt the student to blink while the countdown runs
                    ss << name << " - BLINK NOW! "
                       << std::fixed << std::setprecision(1) << remainingSecs << "s  ("
                       << accuracy << "%)";
                    cv::putText(frame, ss.str(), cv::Point(face.x, face.y - 10),
                                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 255), 2);
                } else {
                    // Blink already detected – show positive feedback during remaining wait
                    ss << name << " - BLINKED! "
                       << std::fixed << std::setprecision(1) << remainingSecs << "s  ("
                       << accuracy << "%)";
                    cv::putText(frame, ss.str(), cv::Point(face.x, face.y - 10),
                                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
                }
            }
            // ── 3-second window over – commit the decision ─────────────────────
            else {
                if (blinked) {
                    // ✅ PRESENT: accuracy >= 30% AND blink verified
                    std::stringstream ss;
                    ss << name << " - PRESENT ("
                       << std::fixed << std::setprecision(1) << accuracy << "%)";
                    cv::putText(frame, ss.str(), cv::Point(face.x, face.y - 10),
                                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
                    logAttendance(personIdx, accuracy, "Present");
                } else {
                    // ❌ ABSENT: accuracy >= 30% but no blink (liveness check failed)
                    std::stringstream ss;
                    ss << name << " - ABSENT (no blink) ("
                       << std::fixed << std::setprecision(1) << accuracy << "%)";
                    cv::putText(frame, ss.str(), cv::Point(face.x, face.y - 10),
                                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
                    logAttendance(personIdx, accuracy, "Absent");
                }

                // Reset stabilisation so the person can be re-checked if they move away
                stabilizationActive_[personIdx] = false;
            }
        } // end face loop

        // ── Reset stabilisation for persons not seen this frame ───────────────
        // If someone walks out of frame for >30 consecutive frames (~1 second)
        // we reset their 3-second window so they get a fresh attempt when they return.
        for (auto& [idx, missedCount] : framesNotSeen_) {
            if (!seenThisFrame.count(idx)) {
                missedCount++;
                if (missedCount > 30) {
                    stabilizationActive_[idx] = false;
                    blinkState_[idx]          = 0;
                }
            }
        }
        // Also initialise framesNotSeen_ entry for newly seen people
        for (int idx : seenThisFrame) {
            if (!framesNotSeen_.count(idx)) framesNotSeen_[idx] = 0;
        }

        // ── HUD: model count + face count overlay ─────────────────────────────
        std::stringstream hud;
        hud << "Models: " << models_.size()
            << "  |  Faces in frame: " << faces.size()
            << "  |  Press 'q' to exit";
        cv::putText(frame, hud.str(), cv::Point(8, 26),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(180, 180, 180), 1);

        cv::imshow("SmartAttendance - Scanner", frame);

        if (cv::waitKey(30) == 'q' ||
            cv::getWindowProperty("SmartAttendance - Scanner", cv::WND_PROP_VISIBLE) < 1)
        {
            break;
        }
    }
}
