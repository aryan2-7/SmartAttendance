#ifndef STUDENTREGISTRATIONWINDOW_H
#define STUDENTREGISTRATIONWINDOW_H

#include <QWidget>
#include <QTimer>
#include <opencv2/core.hpp>
#include <opencv2/objdetect/face.hpp>
#include <opencv2/videoio.hpp>

class QLabel;
class QLineEdit;
class QPushButton;

class StudentRegistrationWindow : public QWidget
{
    Q_OBJECT

public:
    explicit StudentRegistrationWindow(QWidget *parent = nullptr);
    ~StudentRegistrationWindow();

private slots:
    void onRegisterClicked();
    void onFrameTimer();

private:
    void setupUI();
    void saveEmbeddings(const QString &name, const QString &roll);
    bool detectFace(const cv::Mat &frame, cv::Mat &faceBox);

    QLabel* cameraIcon;
    QLabel* statusLabel;
    QLineEdit* nameEdit;
    QLineEdit* rollEdit;
    QPushButton* registerButton;

    cv::VideoCapture cap_;
    QTimer *frameTimer_;
    cv::Ptr<cv::FaceDetectorYN> detector_;
    cv::Ptr<cv::FaceRecognizerSF> recognizer_;

    bool capturing_ = false;
    int sampleCount_ = 0;
    static constexpr int SAMPLES = 50;
    std::vector<cv::Mat> embeddings_;
};

#endif // STUDENTREGISTRATIONWINDOW_H
