#include "WelcomeWindow.h"
#include "FontManager.h"
#include "../theme/Theme.h"
#include "../registration/StudentRegistrationWindow.h"
#include "../attendance/AttendanceWindow.h"
#include "../admin/AdministratorWindow.h"

#include <QApplication>

#include <QFrame>
#include <QGraphicsDropShadowEffect>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QLabel>
#include <QPushButton>

#include <QSpacerItem>
#include <QSvgWidget>



WelcomeWindow::WelcomeWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void WelcomeWindow::setupUI()
{
    // ==========================================================
    // Window
    // ==========================================================

    setWindowTitle("Smart Face Recognition Attendance System");

    resize(1500,900); // fallback size if shown without maximizing

    setMinimumSize(1300,850);

    setObjectName("WelcomeWindow");

    setStyleSheet(QString(R"(

    QWidget#WelcomeWindow
    {
        background:%1;
        color:%2;
    }

    )")
                      .arg(Theme::Card)
                      .arg(Theme::Primary));



    // ==========================================================
    // Main Layout
    // ==========================================================

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->setContentsMargins(70,50,70,40);

    mainLayout->setSpacing(25);



    // ==========================================================
    // TOP BAR
    // ==========================================================

    QHBoxLayout *topBar = new QHBoxLayout();

    topBar->addStretch();



    QLabel *status = new QLabel("  ●  SYSTEM ONLINE  ");

    status->setStyleSheet(QString(R"(

    QLabel{

        background:%1;

        color:%2;

        border:1px solid %3;

        border-radius:15px;

        padding:8px 18px;

        font-size:13px;

        font-weight:600;

    }

    )")
                              .arg(Theme::Surface)
                              .arg(Theme::Success)
                              .arg(Theme::Border));

    topBar->addWidget(status);

    mainLayout->addLayout(topBar);



    // ==========================================================
    // TITLE SECTION
    // ==========================================================

    QWidget *titleWidget = new QWidget();

    QVBoxLayout *titleLayout = new QVBoxLayout(titleWidget);

    titleLayout->setSpacing(5);

    titleLayout->setAlignment(Qt::AlignCenter);



    // Face Icon
    QSvgWidget *icon = new QSvgWidget(":/icons/assets/icons/face.svg");
    icon->setFixedSize(70, 70);

    titleLayout->addWidget(icon, 0, Qt::AlignCenter);




    // Title

    QLabel *title = new QLabel("SMART ATTENDANCE");
    title->setAlignment(Qt::AlignCenter);
    title->setFont(FontManager::titleFont(52));
    title->setStyleSheet(QString("color:%1; background:transparent;").arg(Theme::Primary));
    titleLayout->addWidget(title);

    // Subtitle
    QLabel *subtitle = new QLabel("Face Recognition Attendance System");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setFont(FontManager::appFont(18));
    subtitle->setStyleSheet(QString("color:%1; background:transparent;").arg(Theme::Secondary));
    titleLayout->addWidget(subtitle);

    // Tagline
    QLabel *tagline = new QLabel("Secure  •  Contactless  •  Intelligent");
    tagline->setAlignment(Qt::AlignCenter);
    tagline->setFont(FontManager::appFont(15));
    tagline->setStyleSheet(QString("color:%1; background:transparent;").arg(Theme::Muted));
    titleLayout->addWidget(tagline);

    // Shadow

    QGraphicsDropShadowEffect *shadow =
        new QGraphicsDropShadowEffect;

    shadow->setBlurRadius(80);

    shadow->setOffset(0,0);

    QColor glow(Theme::Gold);
    glow.setAlpha(150);
    shadow->setColor(glow);

    titleWidget->setGraphicsEffect(shadow);



    mainLayout->addWidget(titleWidget);



    // ==========================================================
    // SPACER
    // ==========================================================

    // ==========================================================
    // ACTION CARDS
    // ==========================================================


    // ==========================================================
    // STUDENT REGISTRATION CARD
    // ==========================================================

    QHBoxLayout *cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(30);
    cardsLayout->setAlignment(Qt::AlignCenter);

    QFrame *studentCard = new QFrame;
    studentCard->setFixedSize(340, 360);

    studentCard->setStyleSheet(QString(R"(
QFrame{
    background:%1;
    border:1px solid %2;
    border-radius:20px;
}

QLabel{
    background:transparent;
    border:none;
    color:%3;
}

QPushButton{
    background:%4;
    color:%5;
    border:none;
    border-radius:12px;
    padding:10px;
}

QPushButton:hover{
    background:%6;
}
)")
                                   .arg(Theme::Surface)
                                   .arg(Theme::Border)
                                   .arg(Theme::Primary)
                                   .arg(Theme::Gold)
                                   .arg(Theme::Card)
                                   .arg(Theme::Warning));

    QVBoxLayout *studentLayout = new QVBoxLayout(studentCard);
    studentLayout->setContentsMargins(25,25,25,25);
    studentLayout->setSpacing(18);
    studentLayout->setAlignment(Qt::AlignCenter);

    QSvgWidget *studentIcon =
        new QSvgWidget(":/icons/assets/icons/person_add.svg");

    studentIcon->setFixedSize(70,70);

    studentLayout->addWidget(studentIcon,0,Qt::AlignCenter);

    QLabel *studentTitle =
        new QLabel("STUDENT REGISTRATION");

    studentTitle->setAlignment(Qt::AlignCenter);
    studentTitle->setFont(FontManager::titleFont(18));

    studentLayout->addWidget(studentTitle);

    QLabel *studentDesc =
        new QLabel("Register new students with facial recognition for secure attendance.");

    studentDesc->setWordWrap(true);
    studentDesc->setAlignment(Qt::AlignCenter);
    studentDesc->setFont(FontManager::appFont(12));

    studentLayout->addWidget(studentDesc);

    QPushButton *studentButton =
        new QPushButton("Get Started");

    studentButton->setMinimumHeight(45);

    studentButton->setStyleSheet(QString(R"(
QPushButton{
    background:%1;
    color:%2;
    border:none;
    border-radius:12px;
    padding:10px;
}
QPushButton:hover{
    background:%3;
}
)")
                                     .arg(Theme::Gold)
                                     .arg(Theme::Card)
                                     .arg(Theme::Warning));

    studentButton->setFont(FontManager::buttonFont(12));

    studentLayout->addWidget(studentButton);



    // ==========================================================
    // MARK ATTENDANCE CARD
    // ==========================================================

    QFrame *attendanceCard = new QFrame;
    attendanceCard->setFixedSize(340, 360);

    attendanceCard->setStyleSheet(QString(R"(
QFrame{
    background:%1;
    border:1px solid %2;
    border-radius:20px;
}

QLabel{
    background:transparent;
    border:none;
    color:%3;
}

QPushButton{
    background:%4;
    color:%5;
    border:none;
    border-radius:12px;
    padding:10px;
}

QPushButton:hover{
    background:%6;
}
)")
                                      .arg(Theme::Surface)
                                      .arg(Theme::Border)
                                      .arg(Theme::Primary)
                                      .arg(Theme::Success)
                                      .arg(Theme::Card)
                                      .arg(Theme::Hover));

    QVBoxLayout *attendanceLayout = new QVBoxLayout(attendanceCard);
    attendanceLayout->setContentsMargins(25,25,25,25);
    attendanceLayout->setSpacing(18);
    attendanceLayout->setAlignment(Qt::AlignCenter);

    QSvgWidget *attendanceIcon =
        new QSvgWidget(":/icons/assets/icons/how_to_reg.svg");

    attendanceIcon->setFixedSize(70,70);

    attendanceLayout->addWidget(attendanceIcon,0,Qt::AlignCenter);

    QLabel *attendanceTitle =
        new QLabel("MARK ATTENDANCE");

    attendanceTitle->setAlignment(Qt::AlignCenter);
    attendanceTitle->setFont(FontManager::titleFont(18));

    attendanceLayout->addWidget(attendanceTitle);

    QLabel *attendanceDesc =
        new QLabel("Scan your face to mark attendance quickly and accurately.");

    attendanceDesc->setWordWrap(true);
    attendanceDesc->setAlignment(Qt::AlignCenter);
    attendanceDesc->setFont(FontManager::appFont(12));

    attendanceLayout->addWidget(attendanceDesc);

    QPushButton *attendanceButton =
        new QPushButton("Mark Attendance");

    attendanceButton->setMinimumHeight(45);
    attendanceButton->setFont(FontManager::buttonFont(12));

    attendanceButton->setStyleSheet(QString(R"(
QPushButton{
    background:%1;
    color:%2;
    border:none;
    border-radius:12px;
    padding:10px;
}
QPushButton:hover{
    background:%3;
}
)")
                                        .arg(Theme::Success)
                                        .arg(Theme::Card)
                                        .arg(Theme::Hover));

    attendanceLayout->addWidget(attendanceButton);

    // ==========================================================
    // ADMINISTRATOR CARD
    // ==========================================================

    QFrame *adminCard = new QFrame;
    adminCard->setFixedSize(340, 360);

    adminCard->setStyleSheet(QString(R"(
QFrame{
    background:%1;
    border:1px solid %2;
    border-radius:20px;
}

QLabel{
    background:transparent;
    border:none;
    color:%3;
}

QPushButton{
    background:%4;
    color:%5;
    border:none;
    border-radius:12px;
    padding:10px;
}

QPushButton:hover{
    background:%6;
}
)")
                                 .arg(Theme::Surface)
                                 .arg(Theme::Border)
                                 .arg(Theme::Primary)
                                 .arg(Theme::Warning)
                                 .arg(Theme::Card)
                                 .arg(Theme::Gold));

    QVBoxLayout *adminLayout = new QVBoxLayout(adminCard);
    adminLayout->setContentsMargins(25,25,25,25);
    adminLayout->setSpacing(18);
    adminLayout->setAlignment(Qt::AlignCenter);

    QSvgWidget *adminIcon =
        new QSvgWidget(":/icons/assets/icons/admin_panel_settings.svg");

    adminIcon->setFixedSize(70,70);

    adminLayout->addWidget(adminIcon,0,Qt::AlignCenter);

    QLabel *adminTitle =
        new QLabel("ADMINISTRATOR");

    adminTitle->setAlignment(Qt::AlignCenter);
    adminTitle->setFont(FontManager::titleFont(18));

    adminLayout->addWidget(adminTitle);

    QLabel *adminDesc =
        new QLabel("Manage students, monitor attendance, and generate reports.");

    adminDesc->setWordWrap(true);
    adminDesc->setAlignment(Qt::AlignCenter);
    adminDesc->setFont(FontManager::appFont(12));

    adminLayout->addWidget(adminDesc);

    QPushButton *adminButton =
        new QPushButton("Open Dashboard");

    adminButton->setMinimumHeight(45);
    adminButton->setFont(FontManager::buttonFont(12));

    adminButton->setStyleSheet(QString(R"(
QPushButton{
    background:%1;
    color:%2;
    border:none;
    border-radius:12px;
    padding:10px;
}

QPushButton:hover{
    background:%3;
}
)")
                                   .arg(Theme::Warning)
                                   .arg(Theme::Card)
                                   .arg(Theme::Gold));

    adminLayout->addWidget(adminButton);




    cardsLayout->addWidget(studentCard);
    cardsLayout->addWidget(attendanceCard);
    cardsLayout->addWidget(adminCard);
    mainLayout->addLayout(cardsLayout);

    //--------------------------------------------------
    // Navigation
    //--------------------------------------------------

    connect(studentButton, &QPushButton::clicked, this, [this]()
            {
                auto *window = new StudentRegistrationWindow();
                window->showMaximized();
                this->close();
            });

    connect(attendanceButton, &QPushButton::clicked, this, [this]()
            {
                auto *window = new AttendanceWindow();
                window->showMaximized();
                this->close();
            });

    connect(adminButton, &QPushButton::clicked, this, [this]()
            {
                auto *window = new AdministratorWindow();
                window->showMaximized();
                this->close();
            });

    mainLayout->addStretch();



    // ==========================================================
    // FOOTER
    // ==========================================================

    QLabel *footer =
        new QLabel("Version 1.0     |     © 2026 Computer Engineering Project");

    footer->setAlignment(Qt::AlignCenter);

    footer->setFont(FontManager::appFont(13));
    footer->setStyleSheet(QString("color:%1;").arg(Theme::Muted));

    mainLayout->addWidget(footer);
}