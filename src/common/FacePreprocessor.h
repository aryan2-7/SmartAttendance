#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>

constexpr int kTemporalWindow = 12;

constexpr int    LBPH_RADIUS    = 2;
constexpr int    LBPH_NEIGHBORS = 8;
constexpr int    LBPH_GRID_X    = 8;
constexpr int    LBPH_GRID_Y    = 8;

constexpr double HAAR_SCALE_FACTOR = 1.05;
constexpr int    HAAR_MIN_NEIGHBORS = 6;
constexpr double DEFAULT_THRESHOLD  = 130.0;

// Head-turn liveness constants (replaces old blink-based liveness)
constexpr int    LIVENESS_TURN_FRAMES        = 5;    // consecutive frames of a correct turn required to confirm
constexpr int    LIVENESS_TIMEOUT_FRAMES     = 150;  // ~5s @ 30fps before re-prompting a new random direction
constexpr double LIVENESS_YAW_TURN_THRESHOLD = 0.15; // deviation from center (0.5) required to count as "turned"

class FacePreprocessor {
public:
    FacePreprocessor();
    ~FacePreprocessor();

    bool initialize(const std::string& lbfModelPath);
    bool isInitialized() const { return initialized_; }

    cv::Mat alignFace(const cv::Mat& grayFrame, const cv::Rect& faceRect);
    cv::Mat preprocess(const cv::Mat& alignedFace);

    // Estimates head yaw from LBF landmarks. Returns false if landmarks
    // couldn't be fit (caller should skip/hold, not fail hard).
    // yawRatioOut ~= 0.5 when facing forward; deviates toward 0 or 1 as the
    // head turns. Sign/direction mapping should be verified empirically —
    // see comment in AttendanceMarker.cpp where this is consumed.
    bool computeYawRatio(const cv::Mat& grayFrame, const cv::Rect& faceRect, double& yawRatioOut);

    static const cv::Size kFaceCropSize;

private:
    cv::Ptr<cv::face::Facemark> facemark_;
    bool initialized_;
};
