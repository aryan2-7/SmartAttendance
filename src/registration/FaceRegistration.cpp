#include "FaceRegistration.h"

#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QPixmap>
#include <QRegularExpression>

#include <opencv2/face.hpp>

FaceRegistration::FaceRegistration(QWidget* parent): QWidget(parent), capturing_(false), sampleCount_(0), batchCount_(0){
    buildUI();

    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &FaceRegistration::processFrame);

    preprocessor_.initialize(LBF_MODEL_PATH);

    if (!openCamera()) {
        statusLabel_->setText("Could not open webcam or cascade file.");
        startBtn_->setEnabled(false);
    } else {
        statusLabel_->setText("Enter student details, then click Start Registration.");
        timer_->start(33);
    }
}

FaceRegistration::~FaceRegistration(){
    timer_->stop();
    if (cap_.isOpened())
        cap_.release();
}


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
    cancelBtn_->setEnabled(false);
    
    continueBtn_ = new QPushButton("Continue — I've repositioned", this);
    continueBtn_->setEnabled(false);
    continueBtn_->setStyleSheet(
        "QPushButton { background-color: #1B5E20; border: 2px solid #4CAF50; border-radius: 8px; "
        "padding: 10px; font-size: 14px; font-weight: bold; color: #FFFFFF; }"
        "QPushButton:hover { background-color: #2E7D32; }"
        "QPushButton:pressed { background-color: #4CAF50; color: #121212; }"
        "QPushButton:disabled { background-color: #333; color: #777; border-color: #555; }"
    );
    
    connect(startBtn_, &QPushButton::clicked, this, &FaceRegistration::onStartClicked);
    connect(cancelBtn_, &QPushButton::clicked, this, &FaceRegistration::onCancelClicked);
    connect(continueBtn_, &QPushButton::clicked, this, &FaceRegistration::onContinueClicked);
    
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
    root->addWidget(continueBtn_);
    root->addWidget(progressBar_);
    root->addWidget(statusLabel_);
    setLayout(root);
}


bool FaceRegistration::openCamera(){
    cap_.open(0);
    if (!cap_.isOpened()) {
        qWarning() << "[FaceRegistration] Failed to open camera index 0.";
        return false;
    }
    
    cap_.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap_.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    
    if (!faceCascade_.load(CASCADE_PATH)) {
        qWarning() << "[FaceRegistration] Failed to load cascade file from" << QString::fromStdString(CASCADE_PATH);
        cap_.release();
        return false;
    }
    return true;
}


QString FaceRegistration::getBatchPrompt() const {
    switch (batchCount_) {
        case 0: return "Batch 1/5: Look straight ahead at the camera…";
        case 1: return "Batch 2/5: Turn your head slightly LEFT…";
        case 2: return "Batch 3/5: Turn your head slightly RIGHT…";
        case 3: return "Batch 4/5: Tilt your chin UP…";
        case 4: return "Batch 5/5: Tilt your chin DOWN…";
        default: return "Done!";
    }
}


void FaceRegistration::startNextBatch(){
    capturing_ = true;
    statusLabel_->setText(getBatchPrompt());
}


void FaceRegistration::processFrame(){
    cv::Mat frame;
    if (!cap_.read(frame) || frame.empty()) {
        qWarning() << "[FaceRegistration] Failed to read frame from camera.";
        return;
    }

    cv::flip(frame, frame, 1);

    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    cv::Rect faceRect;
    bool facefound = detectLargestFace(gray, faceRect);

    if(facefound){
        cv::rectangle(frame, faceRect, cv::Scalar(0, 0, 255), 2);
        if (capturing_) {
            cv::Mat alignedFace = preprocessor_.alignFace(gray, faceRect);
            cv::Mat processed = preprocessor_.preprocess(alignedFace);
            faceImages_.push_back(processed);
            faceLabels_.push_back(1);
            
            sampleCount_++;
            progressBar_->setValue(sampleCount_);
            
            int batchSizes[] = { BATCH_FRONT, BATCH_LEFT, BATCH_RIGHT, BATCH_UP, BATCH_DOWN };
            int currentBatchSize = batchSizes[batchCount_];
            int prevBatchTotal = 0;
            for (int i = 0; i < batchCount_; ++i) prevBatchTotal += batchSizes[i];
            int inBatch = sampleCount_ - prevBatchTotal;

            statusLabel_->setText(QString("Capturing... (%1/%2) %3 [sample %4/%5 in batch]")
                .arg(sampleCount_).arg(SAMPLES).arg(getBatchPrompt())
                .arg(inBatch).arg(currentBatchSize));

            std::string overlay = std::to_string(sampleCount_) + " / " + std::to_string(SAMPLES);
            cv::putText(frame, overlay, cv::Point(faceRect.x, faceRect.y - 8), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);

            // End of a batch?
            if (inBatch >= currentBatchSize) {
                capturing_ = false;
                batchCount_++;

                if (sampleCount_ >= SAMPLES) {
                    // All batches done — train
                    timer_->stop();
                    statusLabel_->setText("Training model.. please wait");
                    QApplication::processEvents();

                    QString name = nameEdit_->text().trimmed();
                    QString rollNo = rollEdit_->text().trimmed();

                    QDir().mkpath(QString::fromStdString(MODELS_DIR));
                    std::string modelFileName = buildModelFileName(name, rollNo);
                    std::string modelPath     = MODELS_DIR + modelFileName;

                    bool isUpdate = QFileInfo::exists(QString::fromStdString(modelPath));

                    if (trainAndSave(modelPath)) {
                        QString msg = isUpdate
                            ? QString("Re-registered: %1 (model updated)").arg(name)
                            : QString("Registered: %1").arg(name);
                        statusLabel_->setText(msg);

                        emit registrationComplete(1, name, rollNo);

                        capturing_    = false;
                        sampleCount_  = 0;
                        batchCount_   = 0;
                        faceImages_.clear();
                        faceLabels_.clear();

                        progressBar_->setValue(0);
                        nameEdit_->clear();
                        rollEdit_->clear();
                        nameEdit_->setEnabled(true);
                        rollEdit_->setEnabled(true);
                        startBtn_->setEnabled(true);
                        cancelBtn_->setEnabled(false);
                        continueBtn_->setEnabled(false);
                    }
                    else {
                        statusLabel_->setText("Registration failed try again later");
                        onCancelClicked();
                    }

                    timer_->start(33);
                }
                else {
                    // More batches remain — pause and wait for Continue
                    continueBtn_->setEnabled(true);
                    continueBtn_->setFocus();
                    QString nextPrompt = getBatchPrompt();
                    statusLabel_->setText(QString("Batch %1/%2 done! %3\nWhen ready, click Continue.")
                        .arg(batchCount_).arg(NUM_BATCHES).arg(nextPrompt));
                }
            }
        }
    } else {
        if (capturing_){
            statusLabel_->setText("No face detected. Please adjust your position.");
            cv::putText(frame, "No face detected", cv::Point(20, 40), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
        }
    }

    QImage qimg = matToQImage(frame);
    videoLabel_->setPixmap(QPixmap::fromImage(qimg));
}


bool FaceRegistration::detectLargestFace(const cv::Mat& gray, cv::Rect& faceRect){
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    cv::Mat enhanced;
    clahe->apply(gray, enhanced);

    std::vector<cv::Rect> faces;
    faceCascade_.detectMultiScale(enhanced, faces, 1.05, 6, 0, cv::Size(80, 80));

    if (faces.empty())
        return false;

    faceRect = *std::max_element(faces.begin(), faces.end(),
        [](const cv::Rect& a, const cv::Rect& b) {
            return a.area() < b.area();
        });

    return true;
}


bool FaceRegistration::trainAndSave(const std::string& modelPath){
    if (faceImages_.empty() || faceLabels_.empty()) {
        qWarning() << "[FaceRegistration] No samples to train on.";
        return false;
    }

    try {
        auto recogniser = cv::face::LBPHFaceRecognizer::create(
            LBPH_RADIUS,
            LBPH_NEIGHBORS,
            LBPH_GRID_X,
            LBPH_GRID_Y
        );

        recogniser->train(faceImages_, faceLabels_);
        recogniser->save(modelPath);

        qDebug() << "[FaceRegistration] Model saved to:" << QString::fromStdString(modelPath);
        return true;

    } catch (const cv::Exception& e) {
        qWarning() << "[FaceRegistration] OpenCV exception during training:"
                   << e.what();
        return false;
    }
}


std::string FaceRegistration::buildModelFileName(const QString& name, const QString& roll){
    QString safeName = name;
    safeName.replace(' ', '_');
    safeName.remove(QRegularExpression("[^A-Za-z0-9_\\-]")); 

    QString safeRoll = roll;
    safeRoll.remove(QRegularExpression("[^A-Za-z0-9_\\-]"));

    if (safeName.isEmpty())
        safeName = "unknown";

    return (safeName + "_" + safeRoll + ".yml").toStdString();
}


QImage FaceRegistration::matToQImage(const cv::Mat& mat){
    if (mat.type() == CV_8UC3) {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888).copy();
    }
    
    else if (mat.type() == CV_8UC1) {
        return QImage(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Grayscale8).copy();
    }

    else {
        qWarning() << "[FaceRegistration] Unsupported Mat type for conversion to QImage:" << mat.type();
        return QImage();
    }
}


void FaceRegistration::onStartClicked(){
    QString name   = nameEdit_->text().trimmed();
    QString rollNo = rollEdit_->text().trimmed();

    if (name.isEmpty() || rollNo.isEmpty()) {
        QMessageBox::warning(this, "Missing details", "Please enter naam and a roll number yrrr");
        return;
    }

    faceImages_.clear();
    faceLabels_.clear();
    sampleCount_ = 0;
    batchCount_  = 0;
    progressBar_->setValue(0);

    nameEdit_->setEnabled(false);
    rollEdit_->setEnabled(false);
    startBtn_->setEnabled(false);
    cancelBtn_->setEnabled(true);
    continueBtn_->setEnabled(false);

    capturing_ = true;
    statusLabel_->setText(getBatchPrompt());
}


void FaceRegistration::onContinueClicked(){
    continueBtn_->setEnabled(false);
    startNextBatch();
}


void FaceRegistration::onCancelClicked(){
    capturing_   = false;
    sampleCount_ = 0;
    batchCount_  = 0;
    faceImages_.clear();
    faceLabels_.clear();

    progressBar_->setValue(0);
    nameEdit_->setEnabled(true);
    rollEdit_->setEnabled(true);
    startBtn_->setEnabled(true);
    cancelBtn_->setEnabled(false);
    continueBtn_->setEnabled(false);
    statusLabel_->setText("Registration cancelled. Enter details and try again.");
}
