#include "AttendanceWindow.h"
#include "../auth/FontManager.h"
#include "../auth/WelcomeWindow.h"
#include "../theme/Theme.h"

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
    setObjectName("AttendanceWindow");
    setStyleSheet(QString("QWidget#AttendanceWindow{background:%1;}").arg(Theme::Card));

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
        QString("QPushButton{"
                "background:%1;"
                "color:%2;"
                "border:1px solid %3;"
                "border-radius:10px;"
                "padding:8px;"
                "}"
                "QPushButton:hover{"
                "background:%4;"
                "}")
            .arg(Theme::Surface)
            .arg(Theme::Primary)
            .arg(Theme::Border)
            .arg(Theme::Hover)
        );

    QLabel *title = new QLabel("Smart Attendance");
    title->setFont(FontManager::headingFont(24));
    title->setStyleSheet(QString("color:%1;").arg(Theme::Gold));
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
    pageTitle->setStyleSheet(QString("color:%1;").arg(Theme::Primary));
    pageTitle->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(pageTitle);

    QLabel *dateLabel =
        new QLabel("Date: " + QDate::currentDate().toString("dd MMMM yyyy"));

    dateLabel->setFont(FontManager::appFont(12));
    dateLabel->setStyleSheet(QString("color:%1;").arg(Theme::Secondary));
    dateLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(dateLabel);

    // =========================================================
    // Camera Card
    // =========================================================

    QFrame *cameraCard = new QFrame();
    cameraCard->setFixedSize(540, 500);

    cameraCard->setStyleSheet(
        QString("QFrame{"
                "background:%1;"
                "border:1px solid %2;"
                "border-radius:18px;"
                "}")
            .arg(Theme::Surface)
            .arg(Theme::Border)
        );

    QVBoxLayout *cameraLayout = new QVBoxLayout(cameraCard);
    cameraLayout->setContentsMargins(25,25,25,25);
    cameraLayout->setSpacing(15);
    cameraLayout->setAlignment(Qt::AlignCenter);

    // Camera Placeholder

    QLabel *cameraPlaceholder = new QLabel("👤");

    cameraPlaceholder->setFixedSize(460,345);
    cameraPlaceholder->setAlignment(Qt::AlignCenter);

    cameraPlaceholder->setStyleSheet(
        QString("background:%1;"
                "border:2px dashed %2;"
                "border-radius:20px;"
                "font-size:72px;"
                "color:%3;")
            .arg(Theme::Input)
            .arg(Theme::Gold)
            .arg(Theme::Muted)
        );

    cameraLayout->addStretch();
    cameraLayout->addWidget(cameraPlaceholder,0,Qt::AlignCenter);

    // Ready Badge

    QLabel *statusLabel = new QLabel("🟢 Ready for Recognition");

    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFixedWidth(230);

    statusLabel->setStyleSheet(
        QString("background:%1;"
                "color:%2;"
                "padding:8px;"
                "border-radius:14px;"
                "font-weight:bold;")
            .arg(Theme::Card)
            .arg(Theme::Success)
        );

    cameraLayout->addWidget(statusLabel,0,Qt::AlignCenter);
    cameraLayout->addStretch();

    mainLayout->addWidget(cameraCard,0,Qt::AlignCenter);


    // =========================================================
    // Status Card
    // =========================================================

    QFrame *logCard = new QFrame();
    logCard->setFixedWidth(520);

    logCard->setStyleSheet(
        QString("QFrame{"
                "background:%1;"
                "border:1px solid %2;"
                "border-radius:18px;"
                "}")
            .arg(Theme::Surface)
            .arg(Theme::Border)
        );

    QVBoxLayout *logLayout = new QVBoxLayout(logCard);
    logLayout->setContentsMargins(20,20,20,20);
    logLayout->setSpacing(10);


    QLabel *line1 =
        new QLabel("00:00:00   Ready to mark attendance");
    line1->setStyleSheet(
        QString("color:%1;"
                "font-family:Consolas;"
                "font-size:14px;"
                "font-weight:bold;")
            .arg(Theme::Success)
        );

    logLayout->addWidget(line1);

    mainLayout->addWidget(logCard,0,Qt::AlignCenter);

    setLayout(mainLayout);
}