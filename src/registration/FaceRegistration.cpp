#include "FaceRegistration.h"

#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>



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


