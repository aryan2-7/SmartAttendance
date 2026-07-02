#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <deque>

#include "common/FacePreprocessor.h"

class AttendanceMarker {
public:
    AttendanceMarker();
    ~AttendanceMarker();

    bool initialize(const std::string& cascadePath, const std::string& eyeCascadePath, const std::string& modelPath);
    void setLabelMap(const std::unordered_map<int, std::pair<std::string, std::string>>& map);
    void setThresholds(const std::unordered_map<int, double>& thresholds);
    void run();

private:
    cv::VideoCapture cap_;
    cv::CascadeClassifier faceCascade_;
    cv::CascadeClassifier eyeCascade_;
    cv::Ptr<cv::face::LBPHFaceRecognizer> recognizer_;
    FacePreprocessor preprocessor_;

    std::unordered_map<int, std::pair<std::string, std::string>> labelMap_;
    std::unordered_map<int, int> blinkState_;
    std::unordered_map<int, int> blinkFrameCount_;   // consecutive frames in current state
    std::unordered_map<int, std::chrono::steady_clock::time_point> lastLoggedTime_;

    std::unordered_map<int, double> thresholds_;
    std::deque<int> recentPredictions_;

    // Track who has already been marked present this session
    std::unordered_set<int> loggedLabels_;

    int closeCounter_;

    void logAttendance(int labelId, double accuracy, const std::string& status);
    std::string getCurrentTimestamp();
    int temporalVote();
};
