#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <chrono>

// ─── PersonModel ──────────────────────────────────────────────────────────────
// Holds ONE trained LBPH model and the student info parsed from the filename.
// We need one model per person because every registration saves with label=1,
// so a single recognizer cannot distinguish between people.
struct PersonModel {
    std::string name;
    std::string rollNo;
    cv::Ptr<cv::face::LBPHFaceRecognizer> recognizer;
};


// ─── AttendanceMarker ─────────────────────────────────────────────────────────
class AttendanceMarker {
public:
    AttendanceMarker();
    ~AttendanceMarker();

    // Auto-discovers ALL .yml models from modelsDir and loads them.
    // No manual selection needed – every registered face is checked automatically.
    bool initialize(const std::string& cascadePath,
                    const std::string& eyeCascadePath,
                    const std::string& modelsDir);

    // Starts the main recognition loop (blocks until user presses 'q' or
    // closes the OpenCV window).
    void run();

private:
    cv::VideoCapture        cap_;
    cv::CascadeClassifier   faceCascade_;
    cv::CascadeClassifier   eyeCascade_;

    // One recognizer per registered student (keyed by index into this vector)
    std::vector<PersonModel> models_;

    // ── Per-person state (key = index into models_) ──────────────────────────

    // Blink-detection state machine: 0=init, 1=eyes open, 2=eyes closed, 3=blinked
    std::unordered_map<int, int> blinkState_;

    // Cooldown: when was attendance last written for this person?
    std::unordered_map<int, std::chrono::steady_clock::time_point> lastLoggedTime_;

    // Stabilization: when did this person FIRST appear at >=30% accuracy this session?
    std::unordered_map<int, std::chrono::steady_clock::time_point> stabilizationStart_;
    std::unordered_map<int, bool> stabilizationActive_;

    // How many consecutive frames has this person NOT been seen?
    // If > 30 frames (~1 second), we reset their stabilization so they get
    // a fresh 3-second window when they return.
    std::unordered_map<int, int> framesNotSeen_;

    // ── Helpers ───────────────────────────────────────────────────────────────

    // Runs faceROI against every loaded model and returns the index of the
    // best match (lowest LBPH distance). outConfidence is the LBPH distance.
    // Returns -1 if no models are loaded.
    int predictBestMatch(const cv::Mat& faceROI, double& outConfidence) const;

    // Writes one attendance row to attendance.csv
    void logAttendance(int personIdx, double accuracy, const std::string& status);

    // Returns current wall-clock time as "YYYY-MM-DD HH:MM:SS"
    std::string getCurrentTimestamp();
};
