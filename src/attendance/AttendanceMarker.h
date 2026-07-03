#pragma once
#include <opencv2/core.hpp>
#include <opencv2/objdetect/face.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>
#include <string>
#include <vector>
#include <map>
#include <chrono>

struct StudentRecord {
    std::string name;
    std::string roll;
    cv::Mat     gallery;   // [N × 128] float32 — all enrolled embeddings
};

class AttendanceMarker {
public:
    AttendanceMarker();

    // Call before run(). Loads all .bin files from the gallery directory.
    bool initialize(const std::string &detectorModel,
                    const std::string &recognizerModel,
                    const std::string &galleryDir);

    void run();  // blocking; returns when 'q' pressed or auto-close

private:
    cv::Ptr<cv::FaceDetectorYN>   detector_;
    cv::Ptr<cv::FaceRecognizerSF> recognizer_;
    cv::VideoCapture              cap_;

    std::vector<StudentRecord> students_;

    // Cooldown tracking: roll -> last-logged time
    std::map<std::string, std::chrono::steady_clock::time_point> lastLogged_;

    static constexpr double COSINE_THRESHOLD = 0.363; // SFace recommended threshold
    static constexpr int    COOLDOWN_SECONDS = 300;

    // Blink liveness per detected face slot (index in students_)
    std::map<int, int> blinkState_;
    std::map<int, int> closeCounter_;

    // Frame-skip detection caching
    int     frameCount_   = 0;
    cv::Mat lastFaceBox_;

    // Simple blink state machine for liveness detection.
    // State progression: 0 -> 1 -> 2 -> 3 (blink complete)
    inline int getBlinkState(int current) { return current + 1; }
    inline bool isBlinkComplete(int current) { return current == 3; }

    bool loadGallery(const std::string &galleryDir);
    bool detectBestFace(const cv::Mat &frame, cv::Mat &faceBox);
    int  matchFace(const cv::Mat &feat);   // returns index into students_, or -1
    void logAttendance(const StudentRecord &s, double score, const std::string &status);
    bool livenessCheck(int idx, const cv::Mat &frame, const cv::Mat &faceBox);
};