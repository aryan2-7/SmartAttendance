#include "AttendanceMarker.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <filesystem>
#include <algorithm>

AttendanceMarker::AttendanceMarker()
    : closeCounter_(0), rng_(std::random_device{}()) {
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
        std::cerr << "[Warning] LBF facemark model failed to load — proceeding without alignment or liveness check.\n";
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

std::string AttendanceMarker::pickRandomDirection() {
    std::uniform_int_distribution<int> dist(0, 1);
    return dist(rng_) == 0 ? "LEFT" : "RIGHT";
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
    livenessState_.clear();
    livenessFrameCount_.clear();
    livenessPrompt_.clear();
    livenessTimeoutCounter_.clear();
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

            // Already confirmed + logged this session — no need to keep running liveness
            if (loggedLabels_.find(stableLabel) != loggedLabels_.end()) {
                std::stringstream uiText;
                uiText << name << " - PRESENT (" << std::fixed << std::setprecision(1) << accuracy << "%)";
                cv::putText(frame, uiText.str(), cv::Point(face.x, face.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
                closeCounter_++;
                if (closeCounter_ > 45) {
                    return;
                }
                continue;
            }

            // If no facemark model is loaded, we can't estimate yaw — skip
            // liveness entirely rather than blocking attendance forever
            // (same graceful-degradation philosophy as face alignment).
            if (!preprocessor_.isInitialized()) {
                std::stringstream uiText;
                uiText << name << " - PRESENT (" << std::fixed << std::setprecision(1) << accuracy << "%)";
                cv::putText(frame, uiText.str(), cv::Point(face.x, face.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
                logAttendance(stableLabel, accuracy, "Present");
                loggedLabels_.insert(stableLabel);
                closeCounter_++;
                if (closeCounter_ > 45) {
                    return;
                }
                continue;
            }

            // ── Head-turn liveness test ──────────────────────────────────────
            // Assign a random LEFT/RIGHT prompt the first time this person is
            // seen this session, then require several consecutive frames of a
            // matching head turn (via LBF-landmark yaw estimate) before
            // marking attendance.
            if (livenessPrompt_.find(stableLabel) == livenessPrompt_.end()) {
                livenessPrompt_[stableLabel] = pickRandomDirection();
                livenessState_[stableLabel] = LivenessState::WAITING_TURN;
                livenessFrameCount_[stableLabel] = 0;
                livenessTimeoutCounter_[stableLabel] = 0;
            }

            double yawRatio = 0.5;
            bool yawOk = preprocessor_.computeYawRatio(grayFrame, face, yawRatio);

            std::string direction = livenessPrompt_[stableLabel];
            bool turnedCorrectDirection = false;
            if (yawOk) {
                // NOTE: sign convention (which side is "LEFT" vs "RIGHT") has
                // not been empirically verified against the mirrored preview
                // — flip this comparison if the prompt feels reversed during
                // testing.
                if (direction == "LEFT") {
                    turnedCorrectDirection = (yawRatio - 0.5) < -LIVENESS_YAW_TURN_THRESHOLD;
                } else {
                    turnedCorrectDirection = (yawRatio - 0.5) > LIVENESS_YAW_TURN_THRESHOLD;
                }
            }

            int& lfc = livenessFrameCount_[stableLabel];
            int& timeoutCount = livenessTimeoutCounter_[stableLabel];

            if (turnedCorrectDirection) {
                lfc++;
            } else {
                lfc = 0;
            }
            timeoutCount++;

            if (lfc >= LIVENESS_TURN_FRAMES) {
                livenessState_[stableLabel] = LivenessState::CONFIRMED;
            }

            // If the person doesn't complete the turn in time, re-prompt with
            // a (possibly new) random direction instead of stalling forever.
            if (timeoutCount > LIVENESS_TIMEOUT_FRAMES && livenessState_[stableLabel] != LivenessState::CONFIRMED) {
                livenessPrompt_[stableLabel] = pickRandomDirection();
                direction = livenessPrompt_[stableLabel];
                lfc = 0;
                timeoutCount = 0;
            }

            if (livenessState_[stableLabel] != LivenessState::CONFIRMED) {
                std::stringstream uiText;
                uiText << name << " - TURN HEAD " << direction << " (" << std::fixed << std::setprecision(1) << accuracy << "%)";
                cv::putText(frame, uiText.str(), cv::Point(face.x, face.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);
                closeCounter_ = 0;
            } else {
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
