#pragma once

#include <opencv2/core.hpp>
#include <string>
#include <vector>

enum class LivenessStatus {
    WaitingForFace,
    Prompting,
    Tracking,
    Verified,
    Failed,
    Timeout
};

enum class Direction {
    Up, Down, Left, Right
};

struct DirectionPrompt {
    Direction dir;
    std::string label;
};

class LivenessDetector {
public:
    LivenessDetector();
    bool loadModels(const std::string &modelDir);

    LivenessStatus process(const cv::Mat &frame, const cv::Mat &faceBox);
    void reset();

    std::string statusMessage() const;
    bool isVerified() const;
    Direction currentDirection() const { return currentDir_; }
    int directionProgress() const { return completedDirs_; }
    int totalDirections() const { return totalDirs_; }

private:
    cv::Point2f getFaceCenter(const cv::Mat &faceBox);
    bool checkHeadMovement(const cv::Mat &faceBox);

    void nextDirection();
    std::string directionLabel(Direction d) const;

    LivenessStatus status_;
    Direction currentDir_;
    std::vector<Direction> remainingDirs_;
    int completedDirs_;
    int totalDirs_;
    int trackingFrames_;
    int noFaceFrames_;
    int totalFrames_;

    cv::Point2f refCenter_;
    float refNoseX_;
    bool hasRef_;

    std::string message_;
    bool verified_;
    bool failed_;

    static constexpr float MOVE_THRESHOLD = 25.0f;
    static constexpr int TRACKING_TIMEOUT = 120;
    static constexpr int MIN_DIRECTIONS = 2;
    static constexpr int MAX_DIRECTIONS = 3;
    static constexpr int MAX_NO_FACE = 30;
    static constexpr int MAX_TOTAL_FRAMES = 600;
    static constexpr int SETTLE_FRAMES = 5;
};
