#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <string>
#include <unordered_map>
#include <chrono>

class AttendanceMarker {
public:
    AttendanceMarker();
    ~AttendanceMarker();

    // Initializes the camera, cascade, and LBPH model
    bool initialize(const std::string& cascadePath, const std::string& eyeCascadePath, const std::string& modelPath);

    // Maps integer label ID to a pair of (Name, RollNo)
    void setLabelMap(const std::unordered_map<int, std::pair<std::string, std::string>>& map);

    // Starts the main recognition loop
    void run();

private:
    cv::VideoCapture cap_;
    cv::CascadeClassifier faceCascade_;
    cv::CascadeClassifier eyeCascade_;
    cv::Ptr<cv::face::LBPHFaceRecognizer> recognizer_;

    // Stores ID -> (Name, RollNo)
    std::unordered_map<int, std::pair<std::string, std::string>> labelMap_;

    // Stores ID -> Blink State (0=init, 1=eyes open, 2=eyes closed, 3=blinked)
    std::unordered_map<int, int> blinkState_;

    // To prevent logging the same person 30 times a second, we store the last time they were logged
    std::unordered_map<int, std::chrono::steady_clock::time_point> lastLoggedTime_;

    // Counter to automatically close the window after marking present
    int closeCounter_;

    // Helper to log attendance to CSV
    void logAttendance(int labelId, double accuracy, const std::string& status);

    
    // Helper to get current timestamp as a string
    std::string getCurrentTimestamp();
};
