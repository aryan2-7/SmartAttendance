#include "FacePreprocessor.h"
#include <cmath>
#include <iostream>
#include <fstream>

FacePreprocessor::FacePreprocessor() : initialized_(false) {}

FacePreprocessor::~FacePreprocessor() {}

bool FacePreprocessor::initialize(const std::string& lbfModelPath) {
    // Check file exists before attempting to load
    std::ifstream f(lbfModelPath);
    if (!f.good()) {
        std::cerr << "[FacePreprocessor] LBF model not found at " << lbfModelPath << " — alignment disabled.\n";
        return false;
    }
    f.close();

    try {
        facemark_ = cv::face::createFacemarkLBF();
        facemark_->loadModel(lbfModelPath);
        initialized_ = true;
        return true;
    } catch (const cv::Exception& e) {
        std::cerr << "[FacePreprocessor] Failed to load LBF model: " << e.what() << "\n";
        return false;
    }
}

cv::Mat FacePreprocessor::alignFace(const cv::Mat& grayFrame, const cv::Rect& faceRect) {
    if (!initialized_) {
        return grayFrame(faceRect).clone();
    }

    try {
        cv::Mat grayClone = grayFrame.clone();
        std::vector<cv::Rect> faces = { faceRect };
        std::vector<std::vector<cv::Point2f>> shapes;

        bool success = facemark_->fit(grayClone, faces, shapes);

        if (!success || shapes.empty() || shapes[0].size() < 48) {
            return grayFrame(faceRect).clone();
        }

        const std::vector<cv::Point2f>& lm = shapes[0];

        cv::Point2f leftEye(0, 0), rightEye(0, 0);
        for (int i = 36; i <= 41; ++i) leftEye += lm[i];
        for (int i = 42; i <= 47; ++i) rightEye += lm[i];
        leftEye.x /= 6.0f; leftEye.y /= 6.0f;
        rightEye.x /= 6.0f; rightEye.y /= 6.0f;

        double angle = std::atan2(rightEye.y - leftEye.y, rightEye.x - leftEye.x) * 180.0 / CV_PI;

        cv::Point2f center(faceRect.x + faceRect.width / 2.0f, faceRect.y + faceRect.height / 2.0f);
        cv::Mat M = cv::getRotationMatrix2D(center, angle, 1.0);

        cv::Mat warped;
        cv::warpAffine(grayClone, warped, M, grayClone.size());

        cv::Rect crop(faceRect.x, faceRect.y, faceRect.width, faceRect.height);
        crop &= cv::Rect(0, 0, warped.cols, warped.rows);

        if (crop.width <= 0 || crop.height <= 0) {
            return grayFrame(faceRect).clone();
        }

        return warped(crop).clone();
    } catch (const cv::Exception& e) {
        std::cerr << "[FacePreprocessor] Alignment failed: " << e.what() << " — using raw crop.\n";
        return grayFrame(faceRect).clone();
    }
}

cv::Mat FacePreprocessor::preprocess(const cv::Mat& alignedFace) {
    cv::Mat result;
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(alignedFace, result);

    cv::Mat resized;
    cv::resize(result, resized, kFaceCropSize);
    return resized;
}

bool FacePreprocessor::computeYawRatio(const cv::Mat& grayFrame, const cv::Rect& faceRect, double& yawRatioOut) {
    if (!initialized_) {
        return false;
    }

    try {
        cv::Mat grayClone = grayFrame.clone();
        std::vector<cv::Rect> faces = { faceRect };
        std::vector<std::vector<cv::Point2f>> shapes;

        bool success = facemark_->fit(grayClone, faces, shapes);
        if (!success || shapes.empty() || shapes[0].size() < 48) {
            return false;
        }

        const std::vector<cv::Point2f>& lm = shapes[0];

        // 68-point LBF layout: 36 = left eye outer corner, 45 = right eye
        // outer corner, 30 = nose tip.
        const cv::Point2f& leftCorner  = lm[36];
        const cv::Point2f& rightCorner = lm[45];
        const cv::Point2f& noseTip     = lm[30];

        double eyeSpan = rightCorner.x - leftCorner.x;
        if (std::abs(eyeSpan) < 1e-3) {
            return false;
        }

        // 0.5 = nose centered between the eye corners (facing forward).
        // Moves toward 0 or 1 as the nose shifts relative to the eye line
        // when the head turns.
        yawRatioOut = (noseTip.x - leftCorner.x) / eyeSpan;
        return true;
    } catch (const cv::Exception& e) {
        std::cerr << "[FacePreprocessor] Yaw estimation failed: " << e.what() << "\n";
        return false;
    }
}

const cv::Size FacePreprocessor::kFaceCropSize = cv::Size(200, 200);
