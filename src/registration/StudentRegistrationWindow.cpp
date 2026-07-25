#include "StudentRegistrationWindow.h"
#include "../auth/FontManager.h"
#include "../auth/WelcomeWindow.h"
#include "../theme/Theme.h"
#include "../db/Database.h"
#include "../db/StudentDAO.h"
#include "../db/DbPath.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QSvgWidget>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QPixmap>
#include <QImage>
#include <opencv2/imgproc.hpp>
#include <fstream>
#include <QIntValidator>

static QPixmap matToPixmap(const cv::Mat &frame) {
    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    return QPixmap::fromImage(
        QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy());
}

StudentRegistrationWindow::StudentRegistrationWindow(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setupUI();

    cap_.open(0);
    if (!cap_.isOpened()) {
        statusLabel->setText("Failed to open camera.");
        registerButton->setEnabled(false);
    }

    std::string modelPath = std::string(PROJECT_SOURCE_DIR) + "/resources/models/face_detection_yunet_2023mar.onnx";
    detector_ = cv::FaceDetectorYN::create(modelPath, "", cv::Size(480, 270), 0.9f, 0.3f, 5000);

    frameTimer_ = new QTimer(this);
    connect(frameTimer_, &QTimer::timeout, this, &StudentRegistrationWindow::onFrameTimer);
    frameTimer_->start(33);
}

StudentRegistrationWindow::~StudentRegistrationWindow() {
    frameTimer_->stop();
    if (cap_.isOpened()) cap_.release();
}

void StudentRegistrationWindow::setupUI()
{
    setWindowTitle("Student Registration");
    resize(1400, 850);
    setMinimumSize(1300, 800);
    setObjectName("StudentRegistrationWindow");

    setStyleSheet(QString(R"(
QWidget#StudentRegistrationWindow{
    background:%1;
    color:%2;
}
)")
                      .arg(Theme::Card)
                      .arg(Theme::Primary));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32,24,32,24);
    mainLayout->setSpacing(25);
    mainLayout->setAlignment(Qt::AlignTop);

    QHBoxLayout *headerLayout = new QHBoxLayout();

    QPushButton *backButton = new QPushButton("\u2190 Back");
    connect(backButton, &QPushButton::clicked, this, [this]()
            {
                if (cap_.isOpened()) cap_.release();
                auto *window = new WelcomeWindow();
                window->show();
                this->close();
            });
    backButton->setFixedSize(110,40);

    backButton->setStyleSheet(QString(R"(
QPushButton{
    background:%1;
    color:%2;
    border:1px solid %3;
    border-radius:10px;
}
QPushButton:hover{
    background:%4;
}
)")
                                  .arg(Theme::Surface)
                                  .arg(Theme::Primary)
                                  .arg(Theme::Border)
                                  .arg(Theme::Hover));

    QLabel *title = new QLabel("Student Registration");
    QFont titleFont = FontManager::headingFont();
    titleFont.setPointSize(22);
    title->setFont(titleFont);
    title->setStyleSheet(QString("color:%1;").arg(Theme::Gold));

    headerLayout->addWidget(backButton);
    headerLayout->addSpacing(30);
    headerLayout->addWidget(title);
    headerLayout->addStretch();

    mainLayout->addLayout(headerLayout);

    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(30);
    mainLayout->addLayout(contentLayout);

    // left card: student information form
    QGroupBox *infoBox = new QGroupBox("Student Information");
    infoBox->setFixedWidth(520);

    infoBox->setStyleSheet(QString(R"(
QGroupBox{
    background:%1;
    border:1px solid %2;
    border-radius:15px;
    margin-top:15px;
    font-size:18px;
    font-weight:bold;
    color:%3;
    padding:15px;
}
QGroupBox::title{
    subcontrol-origin: margin;
    left:20px;
    padding:0 8px;
}
QLineEdit{
    background:%4;
    border:1px solid %2;
    border-radius:8px;
    padding:10px;
    color:%3;
}
QLineEdit:focus{
    border:1px solid %5;
}
QLabel{
    background:transparent;
    color:%3;
}
)")
                               .arg(Theme::Surface)
                               .arg(Theme::Border)
                               .arg(Theme::Primary)
                               .arg(Theme::Input)
                               .arg(Theme::Gold));

    auto *infoShadow = new QGraphicsDropShadowEffect;
    infoShadow->setBlurRadius(24);
    infoShadow->setOffset(0, 4);
    infoShadow->setColor(QColor(0, 0, 0, 120));
    infoBox->setGraphicsEffect(infoShadow);

    QFormLayout *formLayout = new QFormLayout(infoBox);
    formLayout->setLabelAlignment(Qt::AlignLeft);
    formLayout->setSpacing(20);
    formLayout->setContentsMargins(20,30,20,20);

    nameEdit = new QLineEdit();
    rollEdit = new QLineEdit();

    nameEdit->setPlaceholderText("Enter Student Name");
    nameEdit->setMinimumWidth(280);
    rollEdit->setPlaceholderText("Enter Roll Number");
    rollEdit->setMinimumWidth(280);
    rollEdit->setValidator(new QIntValidator(1, 99999, this));

    formLayout->addRow("Student Name", nameEdit);
    formLayout->addRow("Roll Number", rollEdit);

    registerButton = new QPushButton("Register Student");
    registerButton->setMinimumHeight(45);
    registerButton->setStyleSheet(QString(R"(
QPushButton{
    background:%1;
    color:%2;
    border:none;
    border-radius:10px;
    font-size:15px;
    font-weight:bold;
}
QPushButton:hover{
    background:%3;
}
)")
                                      .arg(Theme::Gold)
                                      .arg(Theme::Card)
                                      .arg(Theme::Warning));

    connect(registerButton, &QPushButton::clicked, this, &StudentRegistrationWindow::onRegisterClicked);
    formLayout->addRow(registerButton);

    // Continue to Home button
    backHomeButton = new QPushButton("Continue to Home");
    backHomeButton->setMinimumHeight(45);
    backHomeButton->setEnabled(false);

    backHomeButton->setStyleSheet(QString(R"(
QPushButton{
    background:%1;
    color:%2;
    border:1px solid %3;
    border-radius:10px;
    font-size:15px;
    font-weight:bold;
}
QPushButton:hover{
    background:%4;
}
QPushButton:disabled{
    background:%5;
    color:%6;
    border:1px solid %3;
}
)")
                                      .arg(Theme::Surface)
                                      .arg(Theme::Primary)
                                      .arg(Theme::Border)
                                      .arg(Theme::Hover)
                                      .arg(Theme::Input)
                                      .arg(Theme::Secondary));

    // Connect button to WelcomeWindow
    connect(backHomeButton, &QPushButton::clicked, this, [this]()
            {
                if (cap_.isOpened())
                    cap_.release();

                auto *window = new WelcomeWindow();
                window->show();

                this->close();
            });

    formLayout->addRow(backHomeButton);

    backHomeButton->setStyleSheet(QString(R"(
QPushButton{
    background:%1;
    color:%2;
    border:1px solid %3;
    border-radius:10px;
    font-size:15px;
    font-weight:bold;
}
QPushButton:hover{
    background:%4;
}
QPushButton:disabled{
    background:%5;
    color:%6;
    border:1px solid %3;
}
)")
                                      .arg(Theme::Surface)
                                      .arg(Theme::Primary)
                                      .arg(Theme::Border)
                                      .arg(Theme::Hover)
                                      .arg(Theme::Input)
                                      .arg(Theme::Secondary));

    connect(backHomeButton, &QPushButton::clicked, this, [this]()
            {
                if (cap_.isOpened())
                    cap_.release();

                auto *window = new WelcomeWindow();
                window->show();
                this->close();
            });

    formLayout->addRow(backHomeButton);

    // right card: face capture
    QFrame *rightCover = new QFrame;
    rightCover->setFixedWidth(540);

    rightCover->setStyleSheet(QString(R"(
QFrame{
    background:%1;
    border:1px solid %2;
    border-radius:20px;
}
)")
                                  .arg(Theme::Card)
                                  .arg(Theme::Border));

    auto *rightShadow = new QGraphicsDropShadowEffect;
    rightShadow->setBlurRadius(24);
    rightShadow->setOffset(0,4);
    rightShadow->setColor(QColor(0,0,0,120));
    rightCover->setGraphicsEffect(rightShadow);

    QVBoxLayout *rightLayout = new QVBoxLayout(rightCover);
    rightLayout->setContentsMargins(30,30,30,30);
    rightLayout->setSpacing(20);
    rightLayout->setAlignment(Qt::AlignTop);

    QLabel *coverTitle = new QLabel("Face Registration");
    coverTitle->setFont(FontManager::headingFont(18));
    coverTitle->setAlignment(Qt::AlignCenter);
    coverTitle->setStyleSheet(QString("color:%1;").arg(Theme::Primary));
    rightLayout->addWidget(coverTitle);

    QFrame *cameraFrame = new QFrame();
    cameraFrame->setFixedSize(480, 270);

    cameraFrame->setStyleSheet(QString(R"(
QFrame{
    background:%1;
    border:2px solid %2;
    border-radius:20px;
}
)")
                                    .arg(Theme::Input)
                                    .arg(Theme::Border));

    QVBoxLayout *cameraLayout = new QVBoxLayout(cameraFrame);
    cameraLayout->setAlignment(Qt::AlignCenter);

    cameraIcon = new QLabel("\U0001F464");
    cameraIcon->setAlignment(Qt::AlignCenter);
    cameraIcon->setFixedSize(480, 270);

    cameraLayout->addStretch();
    cameraLayout->addWidget(cameraIcon,0,Qt::AlignCenter);

    statusLabel = new QLabel("Ready to Capture");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFixedWidth(300);

    statusLabel->setStyleSheet(QString(R"(
QLabel{
    background:%1;
    color:%2;
    border-radius:12px;
    padding:6px;
    font-weight:bold;
}
)")
                               .arg(Theme::Card)
                               .arg(Theme::Success));

    cameraLayout->addSpacing(15);
    cameraLayout->addWidget(statusLabel,0,Qt::AlignCenter);
    cameraLayout->addStretch();

    rightLayout->addWidget(cameraFrame,0,Qt::AlignCenter);

    QLabel *desc = new QLabel("Position the student's face inside the frame.");
    desc->setAlignment(Qt::AlignCenter);
    desc->setWordWrap(true);
    desc->setFont(FontManager::appFont(11));
    desc->setStyleSheet(QString("color:%1;").arg(Theme::Secondary));
    rightLayout->addWidget(desc);

    contentLayout->addStretch();
    contentLayout->addWidget(infoBox);
    contentLayout->addSpacing(20);
    contentLayout->addWidget(rightCover);
    contentLayout->addStretch();
    contentLayout->setAlignment(Qt::AlignCenter);

    mainLayout->addStretch();
}

void StudentRegistrationWindow::onRegisterClicked() {
    QString name = nameEdit->text().trimmed();
    QString roll = rollEdit->text().trimmed();
    if (name.isEmpty() || roll.isEmpty()) {
        QMessageBox::warning(this, "Missing Info", "Please enter both name and roll number.");
        return;
    }
    bool rollOk;
    int rollNum = roll.toInt(&rollOk);
    if (!rollOk || rollNum < 1) {
        QMessageBox::warning(this, "Invalid Roll", "Roll number must be a positive number.");
        return;
    }

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

    Database db(appDbPath());
    db.initializeTables();
    StudentDAO studentDAO(db.getConnection());
    if (studentDAO.studentExists(rollNum)) {
        QMessageBox::warning(this, "Exists", "Student with this roll number already exists!");
        return;
    }

    sampleCount_ = 0;
    embeddings_.clear();
    capturing_ = true;
    registerButton->setEnabled(false);
    statusLabel->setText("Capturing: 0%");
}

void StudentRegistrationWindow::onFrameTimer() {
    cv::Mat frame;
    cap_ >> frame;
    if (frame.empty()) return;

    cv::flip(frame, frame, 1);

    cv::Mat faceBox;
    if (detector_ && detectFace(frame, faceBox)) {
        int x = static_cast<int>(faceBox.at<float>(0, 0));
        int y = static_cast<int>(faceBox.at<float>(0, 1));
        int w = static_cast<int>(faceBox.at<float>(0, 2));
        int h = static_cast<int>(faceBox.at<float>(0, 3));
        cv::rectangle(frame, {x, y}, {x + w, y + h}, {0, 255, 0}, 2);

        if (capturing_ && recognizer_) {
            cv::Mat aligned, feat;
            recognizer_->alignCrop(frame, faceBox, aligned);
            recognizer_->feature(aligned, feat);
            embeddings_.push_back(feat.clone());
            sampleCount_++;

            statusLabel->setText(QString("Capturing: %1%").arg((sampleCount_ * 100) / SAMPLES));

            if (sampleCount_ >= SAMPLES) {
                capturing_ = false;
                saveEmbeddings(nameEdit->text().trimmed(), rollEdit->text().trimmed());
            }
        }
    } else if (capturing_) {
        statusLabel->setText("No face detected!");
    }

    cv::Mat displayFrame;
    cv::resize(frame, displayFrame, cv::Size(480, 270));
    cameraIcon->setPixmap(matToPixmap(displayFrame));
}

bool StudentRegistrationWindow::detectFace(const cv::Mat &frame, cv::Mat &faceBox) {
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

void StudentRegistrationWindow::saveEmbeddings(const QString &name, const QString &roll) {
    QString modelsDir = QString::fromUtf8(PROJECT_SOURCE_DIR) + "/resources/trained_models/";
    QDir().mkpath(modelsDir);

    QString safeName = name;
    safeName.replace(' ', '_');
    QString filename = modelsDir + safeName + "_" + roll + ".bin";

    cv::Mat gallery;
    cv::vconcat(embeddings_, gallery);

    std::ofstream ofs(filename.toStdString(), std::ios::binary);
    int rows = gallery.rows, cols = gallery.cols;
    ofs.write(reinterpret_cast<char*>(&rows), sizeof(int));
    ofs.write(reinterpret_cast<char*>(&cols), sizeof(int));
    ofs.write(reinterpret_cast<char*>(gallery.data), rows * cols * sizeof(float));
    ofs.close();

    Database db(appDbPath());
    db.initializeTables();
    StudentDAO studentDAO(db.getConnection());
    if (!studentDAO.addStudent(name.toStdString(), roll.toInt(), filename.toStdString())) {
        QFile::remove(filename);
        statusLabel->setText("Registration failed!");
        registerButton->setEnabled(true);
        QMessageBox::critical(this, "Error", "Failed to save student to database.");
        return;
    }

    statusLabel->setText("Registration Complete!");
    registerButton->setEnabled(true);
    backHomeButton->setEnabled(true);
}
