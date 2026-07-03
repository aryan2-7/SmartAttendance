#pragma once
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QTimer>
#include <opencv2/core.hpp>
#include <opencv2/objdetect/face.hpp>
#include <opencv2/videoio.hpp>

class FaceRegistration : public QWidget {
    Q_OBJECT

public:
    explicit FaceRegistration(QWidget *parent = nullptr);
    ~FaceRegistration();

signals:
    void registrationComplete(const QString &name, const QString &rollNo);

private slots:
    void onStartClicked();
    void onFrameTimer();

private:
    // UI
    void buildUI();
    QLabel        *videoLabel_;
    QLineEdit     *nameEdit_;
    QLineEdit     *rollEdit_;
    QPushButton   *startBtn_;
    QProgressBar  *progressBar_;
    QLabel        *statusLabel_;

    // Camera
    cv::VideoCapture cap_;
    QTimer          *frameTimer_;

    // YuNet detector
    cv::Ptr<cv::FaceDetectorYN> detector_;

    // SFace recognizer (used here only for feature extraction)
    cv::Ptr<cv::FaceRecognizerSF> recognizer_;

    // Registration state
    bool            capturing_     = false;
    int             sampleCount_   = 0;
    static constexpr int SAMPLES   = 50;
    std::vector<cv::Mat> embeddings_;   // collected feature vectors

    // Paths — all resources use PROJECT_SOURCE_DIR at runtime (no POST_BUILD needed)

    void saveEmbeddings(const QString &name, const QString &roll);
    bool detectFace(const cv::Mat &frame, cv::Mat &faceBox);
};