#include "AttendanceWindow.h"
#include "../auth/FontManager.h"
#include "../auth/WelcomeWindow.h"
#include "../theme/Theme.h"
#include "../db/db.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QDir>
#include <QPixmap>
#include <QImage>
#include <opencv2/imgproc.hpp>
#include <fstream>
#include <iostream>

static QPixmap matToPixmap(const cv::Mat &frame) {
    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    return QPixmap::fromImage(
        QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy());
}

AttendanceWindow::AttendanceWindow(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    setupUI();

    std::string modelDir = std::string(PROJECT_SOURCE_DIR) + "/resources/models/";
    std::string galleryDir = std::string(PROJECT_SOURCE_DIR) + "/resources/trained_models/";

    detector_ = cv::FaceDetectorYN::create(
        modelDir + "face_detection_yunet_2023mar.onnx", "",
        cv::Size(640, 360), 0.9f, 0.3f, 5000);
    recognizer_ = cv::FaceRecognizerSF::create(
        modelDir + "face_recognition_sface_2021dec.onnx", "");

    if (!loadGallery(galleryDir)) {
        statusLabel->setText("No students registered yet.");
    }

    cap_.open(0);
    if (!cap_.isOpened()) {
        statusLabel->setText("Camera failed to open.");
    }

    frameTimer_ = new QTimer(this);
    connect(frameTimer_, &QTimer::timeout, this, &AttendanceWindow::onFrameTimer);
    frameTimer_->start(33);
}

AttendanceWindow::~AttendanceWindow() {
    frameTimer_->stop();
    if (cap_.isOpened()) cap_.release();
}

void AttendanceWindow::setupUI() {
    setWindowTitle("Attendance Marking");
    resize(1400, 850);
    setStyleSheet(QString("QWidget{ background:%1; color:%2; }")
                      .arg(Theme::Card).arg(Theme::Primary));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(25);
    mainLayout->setAlignment(Qt::AlignTop);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    QPushButton *backButton = new QPushButton("← Back");
    connect(backButton, &QPushButton::clicked, this, [this]() {
        if (cap_.isOpened()) cap_.release();
        auto *window = new WelcomeWindow();
        window->show();
        this->close();
    });
    backButton->setFixedSize(110, 40);
    backButton->setStyleSheet(
        QString("QPushButton{ background:%1; color:%2; border:1px solid %3; "
                "border-radius:10px; } QPushButton:hover{ background:%4; }")
            .arg(Theme::Surface).arg(Theme::Primary)
            .arg(Theme::Border).arg(Theme::Hover));

    QLabel *title = new QLabel("Attendance Marking");
    title->setFont(FontManager::headingFont(22));
    title->setStyleSheet(QString("color:%1;").arg(Theme::Gold));

    headerLayout->addWidget(backButton);
    headerLayout->addSpacing(30);
    headerLayout->addWidget(title);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    QFrame *cameraFrame = new QFrame();
    cameraFrame->setFixedSize(660, 380);
    cameraFrame->setStyleSheet(
        QString("QFrame{ background:%1; border:2px solid %2; border-radius:20px; }")
            .arg(Theme::Input).arg(Theme::Border));

    auto *shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 120));
    cameraFrame->setGraphicsEffect(shadow);

    QVBoxLayout *cameraLayout = new QVBoxLayout(cameraFrame);
    cameraLayout->setAlignment(Qt::AlignCenter);

    cameraIcon = new QLabel("👤");
    cameraIcon->setAlignment(Qt::AlignCenter);
    cameraIcon->setFixedSize(640, 360);
    cameraLayout->addWidget(cameraIcon, 0, Qt::AlignCenter);

    statusLabel = new QLabel("Ready...");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet(
        QString("QLabel{ background:%1; color:%2; border-radius:12px; "
                "padding:6px; font-weight:bold; }")
            .arg(Theme::Surface).arg(Theme::Success));

    mainLayout->addWidget(cameraFrame, 0, Qt::AlignCenter);
    mainLayout->addWidget(statusLabel, 0, Qt::AlignCenter);
    setLayout(mainLayout);
}

bool AttendanceWindow::loadGallery(const std::string &galleryDir) {
    (void)galleryDir;
    Database db(std::string(PROJECT_SOURCE_DIR) + "/smart_attendance.db");
    db.initializeTables();
    std::vector<StudentRecordDB> dbStudents = db.getAllStudents();

    for (const StudentRecordDB &record : dbStudents) {
        if (record.modelPath.empty()) continue;

        std::ifstream ifs(record.modelPath, std::ios::binary);
        if (!ifs) continue;

        int rows = 0, cols = 0;
        ifs.read(reinterpret_cast<char*>(&rows), sizeof(int));
        ifs.read(reinterpret_cast<char*>(&cols), sizeof(int));
        if (rows <= 0 || cols <= 0) continue;

        cv::Mat gallery(rows, cols, CV_32F);
        ifs.read(reinterpret_cast<char*>(gallery.data),
                 static_cast<std::streamsize>(rows * cols * sizeof(float)));
        if (!ifs) continue;

        students_.push_back({
            record.name,
            std::to_string(record.rollNumber),
            gallery.clone()
        });
    }
    return !students_.empty();
}

void AttendanceWindow::onFrameTimer() {
    cv::Mat frame;
    cap_ >> frame;
    if (frame.empty()) return;
    cv::flip(frame, frame, 1);

    cv::Mat faceBox;
    if (detector_ && recognizer_ && detectBestFace(frame, faceBox)) {
        int x = static_cast<int>(faceBox.at<float>(0, 0));
        int y = static_cast<int>(faceBox.at<float>(0, 1));
        int w = static_cast<int>(faceBox.at<float>(0, 2));
        int h = static_cast<int>(faceBox.at<float>(0, 3));
        cv::rectangle(frame, {x, y}, {x + w, y + h}, {0, 255, 0}, 2);

        cv::Mat aligned, feat;
        recognizer_->alignCrop(frame, faceBox, aligned);
        recognizer_->feature(aligned, feat);

        int matchIdx = matchFace(feat);
        if (matchIdx >= 0) {
            auto &s = students_[matchIdx];
            auto now = std::chrono::steady_clock::now();
            bool canLog = true;
            if (lastLogged_.count(s.roll)) {
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                    now - lastLogged_[s.roll]).count();
                if (elapsed < COOLDOWN_SECONDS) canLog = false;
            }
            if (canLog) {
                logAttendance(s);
                lastLogged_[s.roll] = now;
                statusLabel->setText(
                    QString::fromStdString(s.name + " - Attendance Marked!"));
            } else {
                statusLabel->setText(
                    QString::fromStdString(s.name + " - already logged recently."));
            }
        } else {
            statusLabel->setText("Unknown Face");
        }
    } else {
        statusLabel->setText("No face detected.");
    }

    cv::Mat displayFrame;
    cv::resize(frame, displayFrame, cv::Size(640, 360));
    cameraIcon->setPixmap(matToPixmap(displayFrame));
}

bool AttendanceWindow::detectBestFace(const cv::Mat &frame, cv::Mat &faceBox) {
    detector_->setInputSize(frame.size());
    cv::Mat faces;
    detector_->detect(frame, faces);
    if (faces.rows < 1) return false;
    int best = 0;
    for (int i = 1; i < faces.rows; ++i)
        if (faces.at<float>(i, 14) > faces.at<float>(best, 14)) best = i;
    faceBox = faces.row(best);
    return true;
}

int AttendanceWindow::matchFace(const cv::Mat &feat) {
    int bestIdx = -1;
    double bestScore = COSINE_THRESHOLD;
    for (size_t i = 0; i < students_.size(); ++i) {
        for (int r = 0; r < students_[i].gallery.rows; ++r) {
            double score = recognizer_->match(
                feat, students_[i].gallery.row(r),
                cv::FaceRecognizerSF::FR_COSINE);
            if (score > bestScore) {
                bestScore = score;
                bestIdx = static_cast<int>(i);
            }
        }
    }
    return bestIdx;
}

void AttendanceWindow::logAttendance(const StudentRecord &s) {
    Database db(std::string(PROJECT_SOURCE_DIR) + "/smart_attendance.db");
    db.initializeTables();
    db.markAttendance(std::stoi(s.roll), s.name);
}
