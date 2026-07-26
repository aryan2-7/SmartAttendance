//src/attendance/AttendanceWindow.cpp

#include "AttendanceWindow.h"
#include "../auth/FontManager.h"
#include "../auth/WelcomeWindow.h"
#include "../theme/Theme.h"
#include "../db/Database.h"
#include "../db/StudentDAO.h"
#include "../db/AttendanceDAO.h"
#include "../db/SubjectDAO.h"
#include "../db/DbPath.h"

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
#include <ctime>

static QPixmap matToPixmap(const cv::Mat &frame) {
    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    return QPixmap::fromImage(
        QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy());
}

AttendanceWindow::AttendanceWindow(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    setupUI();

    std::string modelDir = bundledResourcesDir().toStdString() + "/models/";

    detector_ = cv::FaceDetectorYN::create(
        modelDir + "face_detection_yunet_2023mar.onnx", "",
        cv::Size(640, 360), 0.9f, 0.3f, 5000);
    recognizer_ = cv::FaceRecognizerSF::create(
        modelDir + "face_recognition_sface_2021dec.onnx", "");

    if (!liveness_.loadModels(modelDir)) {
        statusLabel->setText("Warning: Liveness models not loaded.");
    }

    if (!loadGallery()) {
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
    resize(1400, 850); // fallback size if shown without maximizing
    setStyleSheet(QString("QWidget{ background:%1; color:%2; }")
                      .arg(Theme::Card).arg(Theme::Primary));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(25);
    mainLayout->setAlignment(Qt::AlignTop);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    QPushButton *backButton = new QPushButton("Back");
    connect(backButton, &QPushButton::clicked, this, [this]() {
        if (cap_.isOpened()) cap_.release();
        auto *window = new WelcomeWindow();
        window->showMaximized();
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

    cameraIcon = new QLabel;
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

bool AttendanceWindow::loadGallery() {
    Database dbConn(appDbPath());
    dbConn.initializeTables();
    StudentDAO studentDAO(dbConn.getConnection());
    std::vector<StudentRecord> dbStudents = studentDAO.getAllStudents();

    for (const StudentRecord &record : dbStudents) {
        if (record.studentModelPath.empty()) continue;

        std::ifstream ifs(record.studentModelPath, std::ios::binary);
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
            record.studentId,
            record.studentName,
            std::to_string(record.studentRollNumber),
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
    bool matched = false;

    if (detector_ && recognizer_ && detectBestFace(frame, faceBox)) {
        int x = static_cast<int>(faceBox.at<float>(0, 0));
        int y = static_cast<int>(faceBox.at<float>(0, 1));
        int w = static_cast<int>(faceBox.at<float>(0, 2));
        int h = static_cast<int>(faceBox.at<float>(0, 3));
        cv::rectangle(frame, {x, y}, {x + w, y + h}, {0, 255, 0}, 2);

        if (!livenessPassed_) {
            LivenessStatus ls = liveness_.process(frame, faceBox);
            statusLabel->setText(
                QString::fromStdString(liveness_.statusMessage()));

            if (ls == LivenessStatus::Failed || ls == LivenessStatus::Timeout) {
                liveness_.reset();
                livenessPassed_ = false;
            } else if (ls == LivenessStatus::Verified) {
                livenessPassed_ = true;
            }
        }

        if (livenessPassed_) {
            livenessGraceFrames_ = 0;

            cv::Mat aligned, feat;
            recognizer_->alignCrop(frame, faceBox, aligned);
            recognizer_->feature(aligned, feat);

            int matchIdx = matchFace(feat);
            if (matchIdx >= 0) {
                auto &s = students_[matchIdx];

                auto now = std::chrono::steady_clock::now();
                bool canLog = true;
                if (lastLogged_.count(s.studentId)) {
                    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                        now - lastLogged_[s.studentId]).count();
                    if (elapsed < COOLDOWN_SECONDS) canLog = false;
                }

                if (canLog) {
                    logAttendance(s);
                    lastLogged_[s.studentId] = now;
                } else {
                    statusLabel->setText(
                        QString::fromStdString(s.name + " - already logged recently."));
                }
                matched = true;
            } else {
                statusLabel->setText("Unknown Face");
            }
        }
    } else {
        if (livenessPassed_) {
            livenessGraceFrames_++;
            if (livenessGraceFrames_ > 30) {
                livenessPassed_ = false;
                liveness_.reset();
                livenessGraceFrames_ = 0;
            }
        }
        if (!matched) {
            statusLabel->setText("No face detected.");
        }
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

void AttendanceWindow::logAttendance(const GalleryStudent &s) {
    Database dbConn(appDbPath());
    dbConn.initializeTables();
    AttendanceDAO attendanceDAO(dbConn.getConnection());

    int sessionId = attendanceDAO.findCurrentSession();
    if (sessionId < 0) {
        statusLabel->setText(
            QString::fromStdString(s.name + " - No class scheduled right now."));
        return;
    }

    SubjectDAO subjectDAO(dbConn.getConnection());
    std::vector<SubjectRecord> allSubjects = subjectDAO.getAllSubjects();

    const char* sessionSql = "SELECT sessionSubjectId FROM class_sessions WHERE sessionId = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3* rawDb = dbConn.getConnection();
    int subjectId = -1;

    if (sqlite3_prepare_v2(rawDb, sessionSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, sessionId);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            subjectId = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (subjectId < 0) {
        statusLabel->setText(
            QString::fromStdString(s.name + " - Error: session has no subject."));
        return;
    }

    if (!attendanceDAO.isEnrolled(s.studentId, subjectId)) {
        statusLabel->setText(
            QString::fromStdString(s.name + " - Not enrolled in this subject."));
        return;
    }

    time_t now = time(nullptr);
    char timeBuf[9];
    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", localtime(&now));

    if (attendanceDAO.markAttendance(s.studentId, sessionId, timeBuf, "present")) {
        statusLabel->setText(
            QString::fromStdString(s.name + " - Attendance Marked!"));
    } else {
        statusLabel->setText(
            QString::fromStdString(s.name + " - Already marked for this session."));
    }
}