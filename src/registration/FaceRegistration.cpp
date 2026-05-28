#include "FaceRegistration.h"

#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>

#include <opencv2/face.hpp>

// Here we will make a constructor and destructor for the FaceRegistration class

FaceRegistration::FaceRegistration(QWidget* parent): QWidget(parent), capturing_(false), sampleCount_(0){
    buildUI();

    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &FaceRegistration::processFrame);

    if (!openCamera()) {
        statusLabel_->setText("Could not open webcam or cascade file."); //Either Permissions issue or no webcam or cascade file missing
        startBtn_->setEnabled(false);
    } else {
        statusLabel_->setText("Enter student details, then click Start Registration.");
        timer_->start(33); //Here 33 is abt 30fps
    }
}

FaceRegistration::~FaceRegistration(){
    timer_->stop();
    if (cap_.isOpened())
        cap_.release(); //This line is important to free the webcam after use
}


//Now UI stuff

void FaceRegistration::buildUI(){
    videoLabel_ = new QLabel(this);
    videoLabel_->setFixedSize(640, 480);
    videoLabel_->setAlignment(Qt::AlignCenter);
    videoLabel_->setStyleSheet("background: #111; border: 1px solid #333;");
    videoLabel_->setText("camera plz work…");
    
    nameEdit_ = new QLineEdit(this);
    nameEdit_->setPlaceholderText("Full name  (e.g. Aryan Khatri)");
    
    rollEdit_ = new QLineEdit(this);
    rollEdit_->setPlaceholderText("Roll number  (e.g. 43)");
    
    startBtn_ = new QPushButton("Start Registration", this);
    cancelBtn_ = new QPushButton("Cancel Registration", this);
    cancelBtn_->setEnabled(false); //Initially disabled until registration starts
    
    connect(startBtn_, &QPushButton::clicked, this, &FaceRegistration::onStartClicked);
    connect(cancelBtn_, &QPushButton::clicked, this, &FaceRegistration::onCancelClicked);
    
    progressBar_ = new QProgressBar(this);
    progressBar_->setRange(0, SAMPLES);
    progressBar_->setValue(0);
    progressBar_->setFormat("Samples: %v / " + QString::number(SAMPLES));
    progressBar_->setTextVisible(true);
    
    statusLabel_ = new QLabel("Initialising…", this);
    statusLabel_->setAlignment(Qt::AlignCenter);
    statusLabel_->setWordWrap(true);
    
    QHBoxLayout* inputRow = new QHBoxLayout();
    inputRow->addWidget(nameEdit_);
    inputRow->addWidget(rollEdit_);
    
    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->addWidget(startBtn_);
    btnRow->addWidget(cancelBtn_);
    
    QVBoxLayout* root = new QVBoxLayout(this);
    root->addWidget(videoLabel_, 0, Qt::AlignHCenter);
    root->addLayout(inputRow);
    root->addLayout(btnRow);
    root->addWidget(progressBar_);
    root->addWidget(statusLabel_);
    setLayout(root); //This sets the layout for the widget
}

//Open camera and load cascade file

bool FaceRegistration::openCamera(){
    cap_.open(0); //Open default camera (index 0)
    if (!cap_.isOpened()) {
        qWarning() << "[FaceRegistration] Failed to open camera index 0.";
        return false;
    }
    
    // Set resolution to 640x480 for all round usage
    cap_.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap_.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    
    if (!faceCascade_.load(CASCADE_PATH)) {
        qWarning() << "[FaceRegistration] Failed to load cascade file from" << QString::fromStdString(CASCADE_PATH);
        cap_.release(); //Release camera if cascade loading fails
        return false;
    }
    return true;
}

//Work for later:
//  -processFrame() to capture frames and detect faces, collect samples, and update UI
//  -detectLargestFace() to find the biggest face in the frame using Haar cascades
//  -preprocessFace() to prepare face images for LBPH training
//  -trainAndSave() To train LBPH recogniser and save the model to disk
//  -buildModelFileName() just to have consitant name_rollnum.yml
//  -matToQImage() to convert OpenCV's BGR Mat to Qt's RGB QImage for display
//  -onStartClicked() to start the registration process and lock inputs
//  -onCancelClicked() to cancel the registration and reset the UI
    
