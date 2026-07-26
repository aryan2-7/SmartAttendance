#include "LivenessDetector.h"
#include <algorithm>
#include <cstdio>
#include <random>

LivenessDetector::LivenessDetector() {
    reset();
}

bool LivenessDetector::loadModels(const std::string & /*modelDir*/) {
    return true;
}

void LivenessDetector::reset() {
    status_ = LivenessStatus::WaitingForFace;
    completedDirs_ = 0;
    trackingFrames_ = 0;
    noFaceFrames_ = 0;
    totalFrames_ = 0;
    hasRef_ = false;
    verified_ = false;
    failed_ = false;
    message_.clear();
    remainingDirs_.clear();
}

bool LivenessDetector::isVerified() const {
    return verified_;
}

std::string LivenessDetector::statusMessage() const {
    return message_;
}

std::string LivenessDetector::directionLabel(Direction d) const {
    switch (d) {
        case Direction::Up:    return "UP";
        case Direction::Down:  return "DOWN";
        case Direction::Left:  return "LEFT";
        case Direction::Right: return "RIGHT";
    }
    return "";
}

cv::Point2f LivenessDetector::getFaceCenter(const cv::Mat &faceBox) {
    float x = faceBox.at<float>(0, 0);
    float y = faceBox.at<float>(0, 1);
    float w = faceBox.at<float>(0, 2);
    float h = faceBox.at<float>(0, 3);
    return cv::Point2f(x + w / 2.0f, y + h / 2.0f);
}

void LivenessDetector::nextDirection() {
    if (remainingDirs_.empty()) {
        remainingDirs_ = {Direction::Up, Direction::Down,
                          Direction::Left, Direction::Right};
        static std::mt19937 rng(std::random_device{}());
        std::shuffle(remainingDirs_.begin(), remainingDirs_.end(), rng);
        static std::uniform_int_distribution<int> dist(MIN_DIRECTIONS, MAX_DIRECTIONS);
        totalDirs_ = dist(rng);
    }

    currentDir_ = remainingDirs_.back();
    remainingDirs_.pop_back();
    completedDirs_++;
    trackingFrames_ = 0;
    hasRef_ = false;
    status_ = LivenessStatus::Prompting;

    char buf[128];
    std::snprintf(buf, sizeof(buf), "Move head %s  (%d/%d)",
                  directionLabel(currentDir_).c_str(),
                  completedDirs_, totalDirs_);
    message_ = buf;
}

bool LivenessDetector::checkHeadMovement(const cv::Mat &faceBox) {
    cv::Point2f center = getFaceCenter(faceBox);
    float noseX = faceBox.at<float>(0, 8);

    if (!hasRef_) {
        refCenter_ = center;
        refNoseX_ = noseX;
        hasRef_ = true;
        return false;
    }

    float dx = center.x - refCenter_.x;
    float dy = center.y - refCenter_.y;

    switch (currentDir_) {
        case Direction::Up:
            return dy < -MOVE_THRESHOLD;
        case Direction::Down:
            return dy > MOVE_THRESHOLD;
        case Direction::Left:
            return dx < -MOVE_THRESHOLD;
        case Direction::Right:
            return dx > MOVE_THRESHOLD;
    }
    return false;
}

LivenessStatus LivenessDetector::process(const cv::Mat & /*frame*/,
                                          const cv::Mat &faceBox) {
    if (verified_) return LivenessStatus::Verified;
    if (failed_)  return LivenessStatus::Failed;

    totalFrames_++;
    if (totalFrames_ > MAX_TOTAL_FRAMES) {
        message_ = "Liveness check timed out.";
        failed_ = true;
        return LivenessStatus::Timeout;
    }

    if (faceBox.empty() || faceBox.rows < 1) {
        noFaceFrames_++;
        if (status_ == LivenessStatus::Tracking ||
            status_ == LivenessStatus::Prompting) {
            if (noFaceFrames_ > MAX_NO_FACE) {
                message_ = "Face lost. Restarting...";
                reset();
                return LivenessStatus::WaitingForFace;
            }
            message_ = "Keep your face visible!";
        } else {
            message_ = "Position your face in the camera.";
        }
        return LivenessStatus::WaitingForFace;
    }
    noFaceFrames_ = 0;

    if (status_ == LivenessStatus::WaitingForFace) {
        if (remainingDirs_.empty()) {
            nextDirection();
        } else {
            nextDirection();
        }
        return LivenessStatus::Prompting;
    }

    if (status_ == LivenessStatus::Prompting) {
        trackingFrames_++;
        if (trackingFrames_ > SETTLE_FRAMES) {
            status_ = LivenessStatus::Tracking;
            hasRef_ = false;
            trackingFrames_ = 0;
        }
        return LivenessStatus::Prompting;
    }

    if (status_ == LivenessStatus::Tracking) {
        trackingFrames_++;

        if (checkHeadMovement(faceBox)) {
            hasRef_ = false;
            trackingFrames_ = 0;

            if (completedDirs_ >= totalDirs_) {
                verified_ = true;
                message_ = "Liveness Verified!";
                return LivenessStatus::Verified;
            } else {
                nextDirection();
                return LivenessStatus::Prompting;
            }
        }

        if (trackingFrames_ > TRACKING_TIMEOUT) {
            char buf[128];
            std::snprintf(buf, sizeof(buf),
                "Move head %s  - not detected yet",
                directionLabel(currentDir_).c_str());
            message_ = buf;
            hasRef_ = false;
            trackingFrames_ = 0;
            return LivenessStatus::Prompting;
        }

        char buf[128];
        std::snprintf(buf, sizeof(buf),
            "Move head %s  (%d/%d)",
            directionLabel(currentDir_).c_str(),
            completedDirs_, totalDirs_);
        message_ = buf;
        return LivenessStatus::Tracking;
    }

    return LivenessStatus::WaitingForFace;
}
