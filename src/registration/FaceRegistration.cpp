#include "FaceRegistration.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDir>
#include <QPixmap>
#include <QImage>
#include <QFileInfo>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <fstream>
#include <iostream>

// ── Helper: convert cv::Mat (BGR) to QPixmap ──────────────────────────────
static QPixmap matToPixmap(const cv::Mat &frame) {
    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    return QPixmap::fromImage(
        QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy());
}

// ── Constructor ───────────────────────────────────────────────────────────
FaceRegistration::FaceRegistration(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Face Registration");
    buildUI();

    cap_.open(0);
    if (!cap_.isOpened()) {
        statusLabel_->setText("Failed to open camera.");
        startBtn_->setEnabled(false);
    }

    // Load YuNet immediately so face boxes appear in preview
    {
        std::string modelPath = std::string(PROJECT_SOURCE_DIR) + "/resources/models/face_detection_yunet_2023mar.onnx";
        detector_ = cv::FaceDetectorYN::create(
            modelPath, "", cv::Size(640, 480), 0.9f, 0.3f, 5000);
        if (!detector_) {
            statusLabel_->setText("Failed to load YuNet model — no face boxes.");
        }
    }

    frameTimer_ = new QTimer(this);
    connect(frameTimer_, &QTimer::timeout, this, &FaceRegistration::onFrameTimer);
    frameTimer_->start(33); // ~30 FPS — always running while window is open
}

FaceRegistration::~FaceRegistration() {
    frameTimer_->stop();
    if (cap_.isOpened()) cap_.release();
}

// ── UI ────────────────────────────────────────────────────────────────────
void FaceRegistration::buildUI() {
    auto *root = new QVBoxLayout(this);

    videoLabel_ = new QLabel;
    videoLabel_->setFixedSize(640, 480);
    videoLabel_->setStyleSheet("background: #1a1a1a;");
    videoLabel_->setAlignment(Qt::AlignCenter);
    root->addWidget(videoLabel_);

    auto *formRow = new QHBoxLayout;
    nameEdit_ = new QLineEdit; nameEdit_->setPlaceholderText("Full Name");
    rollEdit_ = new QLineEdit; rollEdit_->setPlaceholderText("Roll No");
    formRow->addWidget(nameEdit_);
    formRow->addWidget(rollEdit_);
    root->addLayout(formRow);

    progressBar_ = new QProgressBar;
    progressBar_->setRange(0, SAMPLES);
    progressBar_->setValue(0);
    root->addWidget(progressBar_);

    statusLabel_ = new QLabel("Enter name and roll, then click Start.");
    statusLabel_->setAlignment(Qt::AlignCenter);
    root->addWidget(statusLabel_);

    startBtn_ = new QPushButton("Start Registration");
    root->addWidget(startBtn_);
    connect(startBtn_, &QPushButton::clicked, this, &FaceRegistration::onStartClicked);

    setLayout(root);
    resize(660, 600);
}

// ── Start button ──────────────────────────────────────────────────────────
void FaceRegistration::onStartClicked() {
    QString name = nameEdit_->text().trimmed();
    QString roll = rollEdit_->text().trimmed();
    if (name.isEmpty() || roll.isEmpty()) {
        QMessageBox::warning(this, "Missing Info", "Please enter both name and roll number.");
        return;
    }

    // Load SFace (only first time — YuNet was loaded in constructor)
    if (!recognizer_) {
        std::string modelPath = std::string(PROJECT_SOURCE_DIR) + "/resources/models/face_recognition_sface_2021dec.onnx";
        recognizer_ = cv::FaceRecognizerSF::create(modelPath, "");
        if (!recognizer_) {
            QMessageBox::critical(this, "Error", "Failed to load SFace model.");
            return;
        }
    }

    if (!cap_.isOpened()) {
        QMessageBox::critical(this, "Error", "Cannot open webcam.");
        return;
    }

    sampleCount_ = 0;
    embeddings_.clear();
    progressBar_->setValue(0);
    capturing_ = true;
    startBtn_->setEnabled(false);
    statusLabel_->setText("Look at the camera. Collecting samples…");
}

// ── Per-frame slot ────────────────────────────────────────────────────────
void FaceRegistration::onFrameTimer() {
    cv::Mat frame;
    cap_ >> frame;
    if (frame.empty()) return;

    cv::flip(frame, frame, 1); // mirror

    // Always detect and draw the face box (YuNet loaded in constructor)
    cv::Mat faceBox;
    if (detectFace(frame, faceBox)) {
        int x = static_cast<int>(faceBox.at<float>(0, 0));
        int y = static_cast<int>(faceBox.at<float>(0, 1));
        int w = static_cast<int>(faceBox.at<float>(0, 2));
        int h = static_cast<int>(faceBox.at<float>(0, 3));
        cv::rectangle(frame, {x, y}, {x + w, y + h}, {0, 255, 0}, 2);

        // Only collect samples when registration is active
        if (capturing_ && recognizer_) {
            cv::Mat aligned;
            recognizer_->alignCrop(frame, faceBox, aligned);
            cv::Mat feat;
            recognizer_->feature(aligned, feat);
            embeddings_.push_back(feat.clone());
            sampleCount_++;
            progressBar_->setValue(sampleCount_);

            statusLabel_->setText(
                QString("Collected %1 / %2").arg(sampleCount_).arg(SAMPLES));

            if (sampleCount_ >= SAMPLES) {
                capturing_ = false;
                saveEmbeddings(nameEdit_->text().trimmed(), rollEdit_->text().trimmed());
            }
        }
    } else if (capturing_) {
        statusLabel_->setText("No face detected. Look at the camera…");
    }

    videoLabel_->setPixmap(matToPixmap(frame).scaled(
        videoLabel_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

// ── Face detection helper ─────────────────────────────────────────────────
// Returns true and fills faceBox (1×15 float Mat) with the best detection.
bool FaceRegistration::detectFace(const cv::Mat &frame, cv::Mat &faceBox) {
    detector_->setInputSize(frame.size());
    cv::Mat faces;
    detector_->detect(frame, faces);
    if (faces.rows < 1) return false;

    // Pick the detection with the highest confidence (column 14)
    int best = 0;
    for (int i = 1; i < faces.rows; ++i)
        if (faces.at<float>(i, 14) > faces.at<float>(best, 14)) best = i;

    faceBox = faces.row(best);
    return true;
}

// ── Save embeddings to disk ───────────────────────────────────────────────
// Format: binary file "<Name>_<Roll>.bin" containing N×128 float32 matrix.
// Each row is one SFace feature vector.
void FaceRegistration::saveEmbeddings(const QString &name, const QString &roll) {
    QString modelsDir = QString::fromUtf8(PROJECT_SOURCE_DIR) + "/resources/trained_models/";
    QDir().mkpath(modelsDir);

    // Filename: "Aryan_Khatri_43.bin"
    QString safeName = name;
    safeName.replace(' ', '_');
    QString filename = modelsDir + safeName + "_" + roll + ".bin";

    // Stack all embeddings into one Mat (rows = samples, cols = 128)
    cv::Mat gallery;
    cv::vconcat(embeddings_, gallery); // shape: [SAMPLES × 128], type CV_32F

    // Write as raw binary: 4-byte int rows, 4-byte int cols, then float data
    std::ofstream ofs(filename.toStdString(), std::ios::binary);
    int rows = gallery.rows, cols = gallery.cols;
    ofs.write(reinterpret_cast<char*>(&rows), sizeof(int));
    ofs.write(reinterpret_cast<char*>(&cols), sizeof(int));
    ofs.write(reinterpret_cast<char*>(gallery.data),
              static_cast<std::streamsize>(rows * cols * sizeof(float)));
    ofs.close();

    statusLabel_->setText(
        QString("Registered %1 (Roll %2) — camera preview active.").arg(name).arg(roll));
    startBtn_->setEnabled(true);

    emit registrationComplete(name, roll);
}
