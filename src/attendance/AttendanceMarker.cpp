#include "AttendanceMarker.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <filesystem>

AttendanceMarker::AttendanceMarker() : closeCounter_(0) {
    recognizer_ = cv::face::LBPHFaceRecognizer::create();
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

    cap_.open(0);
    if (!cap_.isOpened()) {
        std::cerr << "[Error] Failed to open webcam.\n";
        return false;
    }

    return true;
}

void AttendanceMarker::setLabelMap(const std::unordered_map<int, std::pair<std::string, std::string>>& map) {
    labelMap_ = map;
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
    
    // Check cooldown for this person (5 minutes = 300 seconds)
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
        // Format: Name, Roll No, Accuracy, Status, Timestamp
        file << name << "," << rollNo << ",Accuracy: " << std::fixed << std::setprecision(2) << accuracy << "%," << status << "," << timestamp << "\n";
        file.close();
        
        std::string absPath = std::filesystem::absolute("attendance.csv").string();
        std::cout << ">>> ATTENDANCE RECORDED in: " << absPath << "\n";
        std::cout << "    Student: " << name << " (Roll No: " << rollNo << ") at " << timestamp << " (Status: " << status << ", Accuracy: " << std::fixed << std::setprecision(2) << accuracy << "%)\n";
    } else {
        std::cerr << "[Error] Could not write to attendance.csv\n";
    }
}

void AttendanceMarker::run() {
    std::cout << "\n=================================================\n";
    std::cout << "  SmartAttendance Scanner Active. Press 'q' to exit. \n";
    std::cout << "=================================================\n\n";

    // If attendance.csv is empty, write a header row
    std::ifstream checkFile("attendance.csv");
    if (!checkFile.good() || checkFile.peek() == std::ifstream::traits_type::eof()) {
        std::ofstream file("attendance.csv");
        file << "Name,Roll No,Accuracy,Status,Timestamp\n";
        file.close();
    }
    checkFile.close();

    cv::Mat frame, grayFrame;

    while (true) {
        cap_ >> frame;
        if (frame.empty()) break;

        // Flip horizontally to match the registration image (mirror image)
        cv::flip(frame, frame, 1);

        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(grayFrame, grayFrame);

        std::vector<cv::Rect> faces;
        faceCascade_.detectMultiScale(grayFrame, faces, 1.1, 4, 0, cv::Size(100, 100));

        for (const auto& face : faces) {
            cv::Mat faceROI = grayFrame(face);
            
            // Resize to 100x100 to match the registration image size (crucial for LBPH consistency)
            cv::Mat resizedFace;
            cv::resize(faceROI, resizedFace, cv::Size(100, 100));

            int predictedLabel = -1;
            double confidence = 0.0;

            recognizer_->predict(resizedFace, predictedLabel, confidence);

            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);

            // Convert LBPH distance (lower is better, typically 30-100) to a more realistic accuracy percentage
            double accuracy = 100.0 * (1.0 - (confidence / 130.0));
            accuracy = std::max(0.0, std::min(100.0, accuracy));

            if (predictedLabel == -1 || confidence > 130.0) {
                // Completely unrecognized face (distance too large)
                cv::putText(frame, "Face Not Recognised", cv::Point(face.x, face.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
                continue; // Do not log unknown faces
            }

            std::string name = "Unknown";
            if (labelMap_.find(predictedLabel) != labelMap_.end()) {
                name = labelMap_[predictedLabel].first;
            }

            if (accuracy < 30.0) {
                // If it recognized a registered user but confidence is very low (< 30%)
                std::stringstream uiText;
                uiText << name << " - ABSENT (" << std::fixed << std::setprecision(1) << accuracy << "%)";
                cv::putText(frame, uiText.str(), cv::Point(face.x, face.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
                
                logAttendance(predictedLabel, accuracy, "Absent");
            } else {
                // Accuracy >= 40%. Do Liveness Detection (Blink Test)
                cv::Mat faceROIForEyes = grayFrame(face);
                std::vector<cv::Rect> eyes;
                // Look for eyes in the upper half of the face to reduce false positives
                cv::Mat upperFace = faceROIForEyes(cv::Rect(0, 0, face.width, face.height / 2));
                eyeCascade_.detectMultiScale(upperFace, eyes, 1.1, 3, 0, cv::Size(20, 20));

                int currentState = blinkState_[predictedLabel]; // defaults to 0
                
                // State 0: Waiting for eyes to be clearly open (detect >= 2 eyes)
                if (currentState == 0 && eyes.size() >= 2) {
                    blinkState_[predictedLabel] = 1;
                }
                // State 1: Eyes were open. Now waiting for them to close (detect 0 eyes, meaning a blink or looking away)
                else if (currentState == 1 && eyes.size() == 0) {
                    blinkState_[predictedLabel] = 2;
                }
                // State 2: Eyes were closed. Waiting for them to open again (detect >= 1 eye)
                else if (currentState == 2 && eyes.size() >= 1) {
                    blinkState_[predictedLabel] = 3; // Blink complete!
                }

                if (blinkState_[predictedLabel] < 3) {
                    std::stringstream uiText;
                    uiText << name << " - PLEASE BLINK (" << std::fixed << std::setprecision(1) << accuracy << "%)";
                    cv::putText(frame, uiText.str(), cv::Point(face.x, face.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2); // Yellow text
                    closeCounter_ = 0; // Reset auto-close timer if not verified
                } else {
                    std::stringstream uiText;
                    uiText << name << " - PRESENT (" << std::fixed << std::setprecision(1) << accuracy << "%)";
                    cv::putText(frame, uiText.str(), cv::Point(face.x, face.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2); // Green text
                    
                    logAttendance(predictedLabel, accuracy, "Present");
                    
                    closeCounter_++;
                    if (closeCounter_ > 45) { // Auto-close camera scanner after ~1.5s (45 frames) of green PRESENT text
                        return; // Exits run() and closes the webcam
                    }
                }
            }
        }

        cv::imshow("SmartAttendance - Scanner", frame);

        // Break if 'q' is pressed OR if the user clicks the 'X' to close the window
        if (cv::waitKey(30) == 'q' || cv::getWindowProperty("SmartAttendance - Scanner", cv::WND_PROP_VISIBLE) < 1) {
            break;
        }
    }
}
