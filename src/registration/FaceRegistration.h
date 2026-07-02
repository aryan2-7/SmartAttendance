#pragma once
// Header file for the face registration process
//
// This file will contain the FaceRegistration class which will do the following:
//  1.Initialise the camera and show a preview
//  2.Use haar cascade to detect a face in frame
//  3.Take 30 pictures, of the face convert it into greyscale
//  4.Train an LBPH recogniser on those images
//  5.Save the trained models as .yml files
//
// Libraries needed: OpenCV, Qt(6/5)


#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QProgressBar>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/face.hpp>

#include "common/FacePreprocessor.h"

#include <vector>
#include <string>


static const std::string CASCADE_PATH = "resources/haarcascades/haarcascade_frontalface_default.xml";
static const std::string LBF_MODEL_PATH = "resources/models/lbfmodel.yaml";
static const std::string MODELS_DIR = "resources/trained_models/";

static const int SAMPLES = 30;

// Batch sizes: 10 front, 5 left, 5 right, 5 up, 5 down = 30 total
static const int BATCH_FRONT = 10;
static const int BATCH_LEFT  = 5;
static const int BATCH_RIGHT = 5;
static const int BATCH_UP    = 5;
static const int BATCH_DOWN  = 5;
static const int NUM_BATCHES = 5;


// Main class
class FaceRegistration: public QWidget{
    Q_OBJECT
    
    public:
        explicit FaceRegistration(QWidget *parent = nullptr);
        ~FaceRegistration();
        
    signals:
        void registrationComplete(int userId, const QString &name, const QString &rollNo);
        
    private slots:
        void processFrame();
        void onStartClicked();
        void onCancelClicked();
        void onContinueClicked();
        
    private:
    
    //UI
    QLabel* videoLabel_;
    QLineEdit* nameEdit_;
    QLineEdit* rollEdit_;
    QPushButton* startBtn_;
    QPushButton* cancelBtn_;
    QPushButton* continueBtn_;
    QProgressBar* progressBar_;
    QLabel* statusLabel_;
    
    //Camera stuff
    cv::VideoCapture cap_;
    cv::CascadeClassifier faceCascade_;
    QTimer* timer_;
    
    //Preprocessing
    FacePreprocessor preprocessor_;
    
    //Variables
    bool capturing_;
    int sampleCount_;
    int batchCount_; // which batch (0, 1, 2)
    
    std::vector<cv::Mat> faceImages_;
    std::vector<int> faceLabels_;
    
    //helpers
    bool openCamera();
    bool detectLargestFace(const cv::Mat &frame, cv::Rect &faceRect);
    bool trainAndSave(const std::string &modelPath);
    
    static std::string buildModelFileName(const QString &name, const QString &roll);
    static QImage matToQImage(const cv::Mat &mat);
    
    void buildUI();
    QString getBatchPrompt() const;
    void startNextBatch();
};
