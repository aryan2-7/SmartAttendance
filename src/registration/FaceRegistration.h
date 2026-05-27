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
    //need a bunch of UI elements and stuff work for later
    
        
}
    