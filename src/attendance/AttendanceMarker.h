#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <deque>
#include <random>

#include "common/FacePreprocessor.h"

enum class LivenessState {
    WAITING_TURN,
    CONFIRMED
};

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
    cv::CascadeClassifier eyeCascade_; // kept for API/init-signature compatibility; no longer used for liveness
    cv::Ptr<cv::face::LBPHFaceRecognizer> recognizer_;
    FacePreprocessor preprocessor_;

    std::unordered_map<int, std::pair<std::string, std::string>> labelMap_;

    // Head-turn liveness state (replaces the old blink state machine)
    std::unordered_map<int, LivenessState> livenessState_;
    std::unordered_map<int, int> livenessFrameCount_;      // consecutive frames satisfying the current turn requirement
    std::unordered_map<int, std::string> livenessPrompt_;  // "LEFT" or "RIGHT" — randomly assigned per person per session
    std::unordered_map<int, int> livenessTimeoutCounter_;  // frames elapsed since the current prompt was issued

    std::unordered_map<int, std::chrono::steady_clock::time_point> lastLoggedTime_;

    std::unordered_map<int, double> thresholds_;
    std::deque<int> recentPredictions_;

    // Track who has already been marked present this session
    std::unordered_set<int> loggedLabels_;

    std::mt19937 rng_;

    int closeCounter_;

    void logAttendance(int labelId, double accuracy, const std::string& status);
    std::string getCurrentTimestamp();
    int temporalVote();
    std::string pickRandomDirection();
};
