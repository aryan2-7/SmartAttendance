#include "StudentRegistrationWindow.h"
#include "../auth/FontManager.h"
#include "../auth/WelcomeWindow.h"

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

    setStyleSheet(R"(
QWidget{
    background:#0F172A;
    color:white;
}
)");

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

    backButton->setStyleSheet(R"(
QPushButton{
    background:#1E293B;
    color:white;
    border:1px solid #334155;
    border-radius:10px;
}
QPushButton:hover{
    background:#334155;
}
)");

    QLabel *title = new QLabel("Student Registration");
    QFont titleFont = FontManager::headingFont();
    titleFont.setPointSize(22);
    title->setFont(titleFont);
    title->setStyleSheet("color:white;");

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

    infoBox->setStyleSheet(R"(
QGroupBox{
    background:#1E293B;
    border:1px solid #334155;
    border-radius:15px;
    margin-top:15px;
    font-size:18px;
    font-weight:bold;
    padding:15px;
}

QGroupBox::title{
    subcontrol-origin: margin;
    left:20px;
    padding:0 8px;
}

QLineEdit{
    background:#0F172A;
    border:1px solid #475569;
    border-radius:8px;
    padding:10px;
    color:white;
}
QLineEdit:focus{
    border:1px solid #4FD1FF;
}

QLabel{
    background:transparent;
}
)");

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
    registerButton->setStyleSheet(R"(
QPushButton{
    background:#2563EB;
    color:white;
    border:none;
    border-radius:10px;
    font-size:15px;
    font-weight:bold;
}
QPushButton:hover{
    background:#1D4ED8;
}
)");
    formLayout->addRow(registerButton);

    // ---------- right card: face capture / tip panel ----------
    // ---------- right card: face capture ----------
    QFrame *rightCover = new QFrame;
    rightCover->setFixedWidth(500);

    rightCover->setStyleSheet(R"(
QFrame{
    background:#0F172A;
    border:1px solid #334155;
    border-radius:20px;
}
)");

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
    coverTitle->setStyleSheet("color:white;");
    rightLayout->addWidget(coverTitle);

    // Camera Box
    QFrame *cameraFrame = new QFrame();
    cameraFrame->setFixedSize(460,345);

    cameraFrame->setStyleSheet(R"(
QFrame{
    background:#111827;
    border:2px solid #334155;
    border-radius:20px;
}
)");

    QVBoxLayout *cameraLayout = new QVBoxLayout(cameraFrame);
    cameraLayout->setAlignment(Qt::AlignCenter);

    // Camera Placeholder
    QLabel *cameraIcon = new QLabel("👤");
    cameraIcon->setAlignment(Qt::AlignCenter);
    cameraIcon->setFixedSize(380,285);

    cameraIcon->setStyleSheet(R"(
QLabel{
    background:#1F2937;
    border:2px dashed #38BDF8;
    border-radius:18px;
    font-size:70px;
    color:#94A3B8;
}
)");

    cameraLayout->addStretch();
    cameraLayout->addWidget(cameraIcon,0,Qt::AlignCenter);

    // Status Badge
    QLabel *status = new QLabel("Ready to Capture");
    status->setAlignment(Qt::AlignCenter);
    status->setFixedWidth(180);

    status->setStyleSheet(R"(
QLabel{
    background:#123C2C;
    color:#6EE7B7;
    border-radius:12px;
    padding:6px;
    font-weight:bold;
}
)");

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
    desc->setStyleSheet("color:#CBD5E1;");
    rightLayout->addWidget(desc);

    // Tip Box
    QFrame *tipBox = new QFrame;

    tipBox->setStyleSheet(R"(
QFrame{
    background:#1E293B;
    border:1px solid #334155;
    border-radius:12px;
}
)");

    QVBoxLayout *tipLayout = new QVBoxLayout(tipBox);

    QLabel *tipTitle = new QLabel("TIP");
    tipTitle->setFont(FontManager::headingFont(13));
    tipTitle->setStyleSheet("color:#60A5FA;");

    QLabel *tipText = new QLabel(
        "Ensure the student looks directly at the camera with good lighting."
        );

    tipText->setWordWrap(true);
    tipText->setFont(FontManager::appFont(11));
    tipText->setStyleSheet("color:#94A3B8;");

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