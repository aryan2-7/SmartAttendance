#include "AttendanceWindow.h"
#include "../auth/FontManager.h"
#include "../auth/WelcomeWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDate>
#include <QFrame>

AttendanceWindow::AttendanceWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void AttendanceWindow::setupUI()
{
    // Window Background
    setStyleSheet("background-color:#11182D;");

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 20, 30, 20);
    mainLayout->setSpacing(20);

    // =========================================================
    // Header
    // =========================================================

    QHBoxLayout *headerLayout = new QHBoxLayout();

    QPushButton *backButton = new QPushButton("← Back");
    connect(backButton, &QPushButton::clicked, this, [this]()
            {
                auto *window = new WelcomeWindow();
                window->show();
                this->close();
            });
    backButton->setFixedSize(110, 38);
    backButton->setFont(FontManager::buttonFont(11));

    backButton->setStyleSheet(
        "QPushButton{"
        "background:#232D42;"
        "color:white;"
        "border:1px solid #38435D;"
        "border-radius:10px;"
        "padding:8px;"
        "}"
        "QPushButton:hover{"
        "background:#2E3952;"
        "}"
        );

    QLabel *title = new QLabel("Smart Attendance");
    title->setFont(FontManager::headingFont(24));
    title->setStyleSheet("color:white;");
    title->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(backButton);
    headerLayout->addStretch();
    headerLayout->addWidget(title);
    headerLayout->addStretch();

    mainLayout->addLayout(headerLayout);

    // =========================================================
    // Page Title
    // =========================================================

    QLabel *pageTitle = new QLabel("Mark Attendance");
    pageTitle->setFont(FontManager::headingFont(22));
    pageTitle->setStyleSheet("color:white;");
    pageTitle->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(pageTitle);

    QLabel *dateLabel =
        new QLabel("Date: " + QDate::currentDate().toString("dd MMMM yyyy"));

    dateLabel->setFont(FontManager::appFont(12));
    dateLabel->setStyleSheet("color:#A8B3C7;");
    dateLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(dateLabel);

    // =========================================================
    // Camera Card
    // =========================================================

    QFrame *cameraCard = new QFrame();
    cameraCard->setFixedSize(520, 460);

    cameraCard->setStyleSheet(
        "QFrame{"
        "background:#232D42;"
        "border:1px solid #38435D;"
        "border-radius:18px;"
        "}"
        );

    QVBoxLayout *cameraLayout = new QVBoxLayout(cameraCard);
    cameraLayout->setContentsMargins(25,25,25,25);
    cameraLayout->setSpacing(15);
    cameraLayout->setAlignment(Qt::AlignCenter);

    // Camera Placeholder

    QLabel *cameraPlaceholder = new QLabel("👤");

    cameraPlaceholder->setFixedSize(300,300);
    cameraPlaceholder->setAlignment(Qt::AlignCenter);

    cameraPlaceholder->setStyleSheet(
        "background:#141C33;"
        "border:2px dashed #3467E8;"
        "border-radius:20px;"
        "font-size:72px;"
        "color:#64748B;"
        );

    cameraLayout->addStretch();
    cameraLayout->addWidget(cameraPlaceholder,0,Qt::AlignCenter);

    // Ready Badge

    QLabel *statusLabel = new QLabel("🟢 Ready for Recognition");

    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFixedWidth(230);

    statusLabel->setStyleSheet(
        "background:#163A2B;"
        "color:#6EE7B7;"
        "padding:8px;"
        "border-radius:14px;"
        "font-weight:bold;"
        );

    cameraLayout->addWidget(statusLabel,0,Qt::AlignCenter);
    cameraLayout->addStretch();

    mainLayout->addWidget(cameraCard,0,Qt::AlignCenter);

    // =========================================================
    // Status Title
    // =========================================================

    QLabel *statusTitle = new QLabel("System Status");
    statusTitle->setFont(FontManager::headingFont(15));
    statusTitle->setStyleSheet("color:white;");

    mainLayout->addWidget(statusTitle,0,Qt::AlignCenter);

    // =========================================================
    // Status Card
    // =========================================================

    QFrame *logCard = new QFrame();
    logCard->setFixedWidth(520);

    logCard->setStyleSheet(
        "QFrame{"
        "background:#232D42;"
        "border:1px solid #38435D;"
        "border-radius:18px;"
        "}"
        );

    QVBoxLayout *logLayout = new QVBoxLayout(logCard);
    logLayout->setContentsMargins(20,20,20,20);
    logLayout->setSpacing(10);

    QLabel *line1 =
        new QLabel("00:00:00   Camera initialized");
    line1->setStyleSheet(
        "color:#A8B3C7;"
        "font-family:Consolas;"
        "font-size:14px;"
        );

    QLabel *line2 =
        new QLabel("00:00:00   Waiting for student...");
    line2->setStyleSheet(
        "color:#38BDF8;"
        "font-family:Consolas;"
        "font-size:14px;"
        "font-weight:bold;"
        );

    QLabel *line3 =
        new QLabel("00:00:00   Ready to mark attendance");
    line3->setStyleSheet(
        "color:#4ADE80;"
        "font-family:Consolas;"
        "font-size:14px;"
        "font-weight:bold;"
        );

    logLayout->addWidget(line1);
    logLayout->addWidget(line2);
    logLayout->addWidget(line3);

    mainLayout->addWidget(logCard,0,Qt::AlignCenter);

    setLayout(mainLayout);
}