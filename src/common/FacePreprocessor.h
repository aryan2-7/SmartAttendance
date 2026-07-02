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

class FacePreprocessor {
public:
    FacePreprocessor();
    ~FacePreprocessor();

    bool initialize(const std::string& lbfModelPath);
    cv::Mat alignFace(const cv::Mat& grayFrame, const cv::Rect& faceRect);
    cv::Mat preprocess(const cv::Mat& alignedFace);
    static const cv::Size kFaceCropSize;
    
private:
    cv::Ptr<cv::face::Facemark> facemark_;
    bool initialized_;
};
