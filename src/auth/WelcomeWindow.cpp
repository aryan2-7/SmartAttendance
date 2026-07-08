#include "WelcomeWindow.h"
#include "FontManager.h"
#include "../registration/StudentRegistrationWindow.h"
#include "../attendance/AttendanceWindow.h"
#include "../admin/AdministratorWindow.h"

#include <QApplication>
#include <QFontDatabase>

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

    resize(1500,900);

    setMinimumSize(1300,850);

    setStyleSheet(R"(

    QWidget
    {
        background:#0F172A;
        color:white;
    }

    )");



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

    status->setStyleSheet(R"(

    QLabel{

        background:#1E293B;

        color:#22C55E;

        border:1px solid #334155;

        border-radius:15px;

        padding:8px 18px;

        font-size:13px;

        font-weight:600;

    }

    )");

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
    title->setStyleSheet("color:white; background:transparent;");
    titleLayout->addWidget(title);

    // Subtitle
    QLabel *subtitle = new QLabel("Face Recognition Attendance System");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setFont(FontManager::appFont(18));
    subtitle->setStyleSheet("color:#CBD5E1; background:transparent;");
    titleLayout->addWidget(subtitle);

    // Tagline
    QLabel *tagline = new QLabel("Secure  •  Contactless  •  Intelligent");
    tagline->setAlignment(Qt::AlignCenter);
    tagline->setFont(FontManager::appFont(15));
    tagline->setStyleSheet("color:#94A3B8; background:transparent;");
    titleLayout->addWidget(tagline);

    // Shadow

    QGraphicsDropShadowEffect *shadow =
        new QGraphicsDropShadowEffect;

    shadow->setBlurRadius(80);

    shadow->setOffset(0,0);

    shadow->setColor(QColor(59,130,246,150));

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

    studentCard->setStyleSheet(R"(
QFrame{
    background:#1E293B;
    border:1px solid #334155;
    border-radius:20px;
}

QLabel{
    background:transparent;
    border:none;
    color:white;
}

QPushButton{
    background:#2563EB;
    color:white;
    border:none;
    border-radius:12px;
    padding:10px;
}

QPushButton:hover{
    background:#1D4ED8;
}
)");

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

    studentButton->setStyleSheet(R"(
QPushButton{
    background:#2563EB;
    color:white;
    border:none;
    border-radius:12px;
    padding:10px;
}
QPushButton:hover{
    background:#1D4ED8;
}
)");

    studentButton->setFont(FontManager::buttonFont(12));

    studentLayout->addWidget(studentButton);



    // ==========================================================
    // MARK ATTENDANCE CARD
    // ==========================================================

    QFrame *attendanceCard = new QFrame;
    attendanceCard->setFixedSize(340, 360);

    attendanceCard->setStyleSheet(R"(
QFrame{
    background:#1E293B;
    border:1px solid #334155;
    border-radius:20px;
}

QLabel{
    background:transparent;
    border:none;
    color:white;
}

QPushButton{
    background:#10B981;
    color:white;
    border:none;
    border-radius:12px;
    padding:10px;
}

QPushButton:hover{
    background:#059669;
}
)");

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

    attendanceButton->setStyleSheet(R"(
QPushButton{
    background:#10B981;
    color:white;
    border:none;
    border-radius:12px;
    padding:10px;
}
QPushButton:hover{
    background:#059669;
}
)");

    attendanceLayout->addWidget(attendanceButton);

    // ==========================================================
    // ADMINISTRATOR CARD
    // ==========================================================

    QFrame *adminCard = new QFrame;
    adminCard->setFixedSize(340, 360);

    adminCard->setStyleSheet(R"(
QFrame{
    background:#1E293B;
    border:1px solid #334155;
    border-radius:20px;
}

QLabel{
    background:transparent;
    border:none;
    color:white;
}

QPushButton{
    background:#F59E0B;
    color:white;
    border:none;
    border-radius:12px;
    padding:10px;
}

QPushButton:hover{
    background:#D97706;
}
)");

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

    adminButton->setStyleSheet(R"(
QPushButton{
    background:#F59E0B;
    color:white;
    border:none;
    border-radius:12px;
    padding:10px;
}

QPushButton:hover{
    background:#D97706;
}
)");

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
                window->show();
                this->close();
            });

    connect(attendanceButton, &QPushButton::clicked, this, [this]()
            {
                auto *window = new AttendanceWindow();
                window->show();
                this->close();
            });

    connect(adminButton, &QPushButton::clicked, this, [this]()
            {
                auto *window = new AdministratorWindow();
                window->show();
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
    footer->setStyleSheet("color:#64748B;");

    mainLayout->addWidget(footer);
}