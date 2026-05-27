// Header file for the face registration process
//
// This file will contain the FaceRegistration class which will do the following:
//  1.Initialise the camera and show a prewview
//  2.Use haar cascade to detect a face in frame
//  3.Take 30 pictures, of the face convert it into greyscale
//  4.Train an LBPH recogniser on those images
//  5.Save the trained modles as .yml files
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
#include <opencv2/face.hpp>      // LBPHFaceRecognizer if youre having problems with this, you prob havent installed the opencv_contrib which we NEED

#include <vector>
#include <string>


// Here we are defining where the cascade file is and where the models after training should be saved at
static const std::string CASCADE_PATH = "../resources/haarcascades/haarcascade_frontalface_default.xml";

static const std::string MODELS_DIR = "../resources/trained_models/";

static const int SAMPLES = 30; // We are gonna be taking 30 pictures for the LBPH training


// Main class
class FaceRegistration: public QWidget{
    Q_OBJECT
    
    public:
        explicit FaceRegistration(QWidget *parent = nullptr);
        ~FaceRegistration();
        
    signals: //Sent after all 30 samples are collected to train the LBPH and .yml file creation
        void registrationComplete(int userId, const QString &name, const QString &rollNo);
        
    private slots:
        void processFrame(); // Bacically camera
        
        void onStartClicked(); // Stars the registration process
        
        void onEndClicked(); // Stops the regis. process
        
    private:
    
    //UI
    QLabel* videoLable_; //webcam preview
    QLineEdit* nameEdit_; //student name
    QLineEdit* rollEdit_; //student roll
    QPushButton* startBtn_; //starts process
    QPushButton* cancelBtn_; //ends process
    QProgressBar* progressBar_; //how many pictures
    QLabel* statusLabel_; //text
    
    //Camera stuff
    cv::VideoCapture cap_; //webcam
    cv::CascadeClassifier faceCascade_; //Harr face cascade to recognise what a face is like
    QTimer* timer_; //ticks
    
    //Variables
    bool capturing_; //T if capturing, F if not started or cancelled
    int sampleCount_; //number of samples
    
    std::vector<cv::Mat> faceImage_; // Preprocessed 100x100 greayscale box of sadness
    std::vector<int> faceLabels_; //this js says its one person for .yml
    
    //helpers
    bool openCam();
    bool detectLargestFace(const cv::Mat &frame, cv::Rect &faceRect); //Picks the largest face and draws a rectangle around it
    bool trainAndSave(const std::string &modelPath); //trains and saves gng
    cv::Mat preprocessFace(const cv::Mat &grayFaceCrop); //each of the 30 samples need to be cropped and turned into greyscale for LBPH training this just helps in that
    
    static std::string buildModelFileName(const QString &name, const QString &roll); //this is for the naming style of the .yml models after training
    
    static QImage matToQImage(const cv::Mat &mat); //this is required as opencv and Qt have diff image parameters so we gottta convert
    
    void buildUI(); //this will be called by a constructor
        
};
