#include "StudentRegistrationWindow.h"
#include "../auth/FontManager.h"
#include "../auth/WelcomeWindow.h"
#include "../theme/Theme.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QSvgWidget>
#include <QGraphicsDropShadowEffect>

StudentRegistrationWindow::StudentRegistrationWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void StudentRegistrationWindow::setupUI()
{
    setWindowTitle("Student Registration");
    resize(1400, 850);
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

    QPushButton *backButton = new QPushButton("← Back");
    connect(backButton, &QPushButton::clicked, this, [this]()
            {
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

    // ---------- left card: student information form ----------
    QGroupBox *infoBox = new QGroupBox("Student Information");
    infoBox->setFixedWidth(450);

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


    QLineEdit *nameEdit = new QLineEdit();
    QLineEdit *rollEdit = new QLineEdit();


    nameEdit->setPlaceholderText("Enter Student Name");
    rollEdit->setPlaceholderText("Enter Roll Number");


    formLayout->addRow("Student Name", nameEdit);
    formLayout->addRow("Roll Number", rollEdit);

    QPushButton *registerButton = new QPushButton("Register Student");
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
    formLayout->addRow(registerButton);

    // ---------- right card: face capture / tip panel ----------
    // ---------- right card: face capture ----------
    QFrame *rightCover = new QFrame;
    rightCover->setFixedWidth(500);

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

    // Title
    QLabel *coverTitle = new QLabel("Face Registration");
    coverTitle->setFont(FontManager::headingFont(18));
    coverTitle->setAlignment(Qt::AlignCenter);
    coverTitle->setStyleSheet(QString("color:%1;").arg(Theme::Primary));
    rightLayout->addWidget(coverTitle);

    // Camera Box
    QFrame *cameraFrame = new QFrame();
    cameraFrame->setFixedSize(460,345);

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

    // Camera Placeholder
    QLabel *cameraIcon = new QLabel("👤");
    cameraIcon->setAlignment(Qt::AlignCenter);
    cameraIcon->setFixedSize(380,285);

    cameraIcon->setStyleSheet(QString(R"(
QLabel{
    background:%1;
    border:2px dashed %2;
    border-radius:18px;
    font-size:70px;
    color:%3;
}
)")
                                  .arg(Theme::Surface)
                                  .arg(Theme::Gold)
                                  .arg(Theme::Secondary));

    cameraLayout->addStretch();
    cameraLayout->addWidget(cameraIcon,0,Qt::AlignCenter);

    // Status Badge
    QLabel *status = new QLabel("Ready to Capture");
    status->setAlignment(Qt::AlignCenter);
    status->setFixedWidth(180);

    status->setStyleSheet(QString(R"(
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
    cameraLayout->addWidget(status,0,Qt::AlignCenter);
    cameraLayout->addStretch();

    rightLayout->addWidget(cameraFrame,0,Qt::AlignCenter);

    // Description
    QLabel *desc = new QLabel(
        "Position the student's face inside the frame.\n"
        "The system will capture facial features automatically."
        );

    desc->setAlignment(Qt::AlignCenter);
    desc->setWordWrap(true);
    desc->setFont(FontManager::appFont(11));
    desc->setStyleSheet(QString("color:%1;").arg(Theme::Secondary));
    rightLayout->addWidget(desc);

    // Tip Box
    QFrame *tipBox = new QFrame;

    tipBox->setStyleSheet(QString(R"(
QFrame{
    background:%1;
    border:1px solid %2;
    border-radius:12px;
}
)")
                              .arg(Theme::Surface)
                              .arg(Theme::Border));

    QVBoxLayout *tipLayout = new QVBoxLayout(tipBox);

    QLabel *tipTitle = new QLabel("TIP");
    tipTitle->setFont(FontManager::headingFont(13));
    tipTitle->setStyleSheet(QString("color:%1;").arg(Theme::Gold));

    QLabel *tipText = new QLabel(
        "Ensure the student looks directly at the camera with good lighting."
        );

    tipText->setWordWrap(true);
    tipText->setFont(FontManager::appFont(11));
    tipText->setStyleSheet(QString("color:%1;").arg(Theme::Secondary));

    tipLayout->addWidget(tipTitle);
    tipLayout->addWidget(tipText);

    rightLayout->addWidget(tipBox);

    // ---------- assemble the row: centered pair of cards ----------
    contentLayout->addStretch();
    contentLayout->addWidget(infoBox);
    contentLayout->addSpacing(20);
    contentLayout->addWidget(rightCover);
    contentLayout->addStretch();
    contentLayout->setAlignment(Qt::AlignCenter);

    mainLayout->addStretch();
}