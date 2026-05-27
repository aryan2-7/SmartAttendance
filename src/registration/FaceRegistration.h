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


