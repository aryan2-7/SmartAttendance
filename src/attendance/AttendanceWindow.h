#pragma once

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <opencv2/core.hpp>
#include <opencv2/objdetect/face.hpp>
#include <opencv2/videoio.hpp>
#include <string>
#include <vector>
#include <map>
#include <chrono>

struct GalleryStudent {
    int         studentId;
    std::string name;
    std::string roll;
    cv::Mat     gallery;
};

class AttendanceWindow : public QWidget {
    Q_OBJECT

public:
    explicit AttendanceWindow(QWidget *parent = nullptr);
    ~AttendanceWindow();

private slots:
    void onFrameTimer();

private:
    void setupUI();
    bool loadGallery();
    bool detectBestFace(const cv::Mat &frame, cv::Mat &faceBox);
    int  matchFace(const cv::Mat &feat);
    void logAttendance(const GalleryStudent &s);

    QLabel *cameraIcon;
    QLabel *statusLabel;

    cv::VideoCapture cap_;
    QTimer *frameTimer_;
    cv::Ptr<cv::FaceDetectorYN>   detector_;
    cv::Ptr<cv::FaceRecognizerSF> recognizer_;

    std::vector<GalleryStudent> students_;
    std::map<int, std::chrono::steady_clock::time_point> lastLogged_;
    static constexpr double COSINE_THRESHOLD = 0.363;
    static constexpr int    COOLDOWN_SECONDS = 300;
};


