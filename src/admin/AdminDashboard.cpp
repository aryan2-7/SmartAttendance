#include "AdminDashboard.h"
#include "CircularProgress.h"
#include "../auth/FontManager.h"
#include "../theme/Theme.h"
#include "ManageStudentsWindow.h"
#include "AttendanceRecordsWindow.h"
#include "AdministratorWindow.h"
#include "../db/db.h"
#include "SubjectManagementWindow.h"
#include "ScheduleEditor.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QDate>
#include <QGraphicsDropShadowEffect>
#include <QProgressBar>
#include <QSizePolicy>
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include <algorithm>


AdminDashboard::AdminDashboard(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setupUI();
    refreshDashboard();
}

void AdminDashboard::refreshDashboard()
{
    Database db(std::string(PROJECT_SOURCE_DIR) + "/smart_attendance.db");
    db.initializeTables();
    auto students = db.getAllStudents();
    auto records = db.getAllRecords();

    int totalStudents = static_cast<int>(students.size());

    // Count today's check-ins
    std::string today = QDate::currentDate().toString("yyyy-MM-dd").toStdString();
    int todayCount = 0;
    int lateTally = 0;
    for (auto &r : records) {
        if (r.date == today) {
            ++todayCount;
            if (r.time > "09:00:00") ++lateTally;
        }
    }

    int attendancePct = (totalStudents > 0)
        ? (todayCount * 100) / totalStudents
        : 0;

    if (totalStudents > 0) {
        progressCircle->setPercentage(attendancePct);
        int presentOnTime = todayCount - lateTally;
        presentCount->setText(QString::number(presentOnTime) + " Present");
        lateCount->setText(QString::number(lateTally) + " Late");
        absentCount->setText(QString::number(totalStudents - todayCount) + " Absent");
    } else {
        progressCircle->setPercentage(0);
        presentCount->setText("No Data");
        this->lateCount->setText("");
        absentCount->setText("");
    }

    // Build Recent Check-ins (last 5)
    QLayout *oldLayout = recentCard->layout();
    if (oldLayout) {
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0))) {
            if (item->widget()) delete item->widget();
            delete item;
        }
        delete oldLayout;
    }

    QVBoxLayout *recentLayout = new QVBoxLayout(recentCard);
    recentLayout->setContentsMargins(25, 20, 25, 20);
    recentLayout->setSpacing(15);

    QLabel *recentTitle = new QLabel("Recent Check-ins");
    recentTitle->setFont(FontManager::headingFont(18));
    recentTitle->setStyleSheet(QString("color:%1;").arg(Theme::Primary));
    recentLayout->addWidget(recentTitle);

    int maxRows = (std::min)(5, static_cast<int>(records.size()));
    for (int i = 0; i < maxRows; ++i) {
        auto &r = records[i];
        QFrame *row = new QFrame();
        row->setStyleSheet(QString("QFrame{ background:%1; border-radius:12px; }")
                               .arg(Theme::Input));
        row->setMinimumHeight(50);

        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(20, 10, 20, 10);

        QLabel *avatar = new QLabel("\U0001F464");
        avatar->setStyleSheet("font-size:24px;");
        QLabel *name = new QLabel(QString::fromStdString(r.studentName));
        name->setFont(FontManager::buttonFont(14));
        name->setStyleSheet(QString("color:%1;").arg(Theme::Primary));

        bool isLate = (r.time > "09:00:00");
        QString statusText = isLate ? "Late" : "Present";
        QString statusColor = isLate ? Theme::Warning : Theme::Success;

        QLabel *badge = new QLabel(statusText);
        badge->setStyleSheet(
            QString("background:%1; color:%2; padding:6px 14px; border-radius:10px;")
                .arg(Theme::Card).arg(statusColor));

        QLabel *timeLabel = new QLabel(
            QString::fromStdString(r.time.substr(0, 5)));
        timeLabel->setStyleSheet(QString("color:%1;").arg(Theme::Secondary));

        rowLayout->addWidget(avatar);
        rowLayout->addSpacing(10);
        rowLayout->addWidget(name);
        rowLayout->addStretch();
        rowLayout->addWidget(badge);
        rowLayout->addSpacing(20);
        rowLayout->addWidget(timeLabel);

        recentLayout->addWidget(row);
    }

    if (records.empty()) {
        QLabel *noData = new QLabel("No attendance records yet.");
        noData->setStyleSheet(QString("color:%1;").arg(Theme::Secondary));
        noData->setAlignment(Qt::AlignCenter);
        recentLayout->addWidget(noData);
    }
}

void AdminDashboard::setupUI()
{
    setWindowTitle("Admin Dashboard");
    resize(1400,850);
    setMinimumSize(1300, 800);
    setObjectName("AdminDashboard");

    setStyleSheet(QString(R"(
QWidget#AdminDashboard{
    background:%1;
    color:%2;
}
QLabel{
    background:transparent;
    border:none;
}
QLineEdit{
    background:%3;
    border:1px solid %4;
    border-radius:12px;
    padding:10px;
    color:%2;
    font-size:14px;
}
QLineEdit:focus{
    border:1px solid %5;
}
QPushButton{
    background:%6;
    color:%2;
    border:1px solid %4;
    border-radius:10px;
    padding:10px 20px;
}
QPushButton:hover{
    background:%7;
}
)")
                      .arg(Theme::Card)
                      .arg(Theme::Primary)
                      .arg(Theme::Input)
                      .arg(Theme::Border)
                      .arg(Theme::Gold)
                      .arg(Theme::Surface)
                      .arg(Theme::Hover));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30,25,30,25);
    mainLayout->setSpacing(25);

    // Header
    QHBoxLayout *header = new QHBoxLayout();

    QPushButton *backButton = new QPushButton("← Back");
    connect(backButton, &QPushButton::clicked, this, [this]()
            {
                auto *window = new AdministratorWindow();
                window->show();
                this->close();
            });
    backButton->setFixedSize(105,38);

    QVBoxLayout *titleLayout = new QVBoxLayout();

    QLabel *title = new QLabel("Welcome, Administrator");
    title->setFont(FontManager::headingFont(24));
    title->setStyleSheet(QString("color:%1;").arg(Theme::Gold));

    QLabel *date = new QLabel(
        QDate::currentDate().toString("dddd • MMMM d, yyyy"));
    date->setStyleSheet(QString("color:%1;").arg(Theme::Secondary));

    titleLayout->addWidget(title);
    titleLayout->addWidget(date);

    header->addWidget(backButton);
    header->addSpacing(25);
    header->addLayout(titleLayout);
    header->addStretch();

    mainLayout->addLayout(header);

    // Dashboard Row
    QHBoxLayout *topCards = new QHBoxLayout();
    topCards->setSpacing(20);

    // Attendance Card
    QFrame *attendanceCard = new QFrame();
    attendanceCard->setMinimumSize(400,200);
    attendanceCard->setStyleSheet(QString(R"(
QFrame{
    background:%1;
    border:1px solid %2;
    border-radius:20px;
}
)")
                                      .arg(Theme::Surface)
                                      .arg(Theme::Border));

    auto shadow1 = new QGraphicsDropShadowEffect;
    shadow1->setBlurRadius(25);
    shadow1->setOffset(0,4);
    shadow1->setColor(QColor(0,0,0,140));
    attendanceCard->setGraphicsEffect(shadow1);

    QHBoxLayout *attendanceLayout = new QHBoxLayout(attendanceCard);
    attendanceLayout->setContentsMargins(30,25,30,25);
    attendanceLayout->setSpacing(25);

    // Left Side (Circle progress)
    QVBoxLayout *circleLayout = new QVBoxLayout();
    circleLayout->setAlignment(Qt::AlignCenter);

    progressCircle = new CircularProgress();
    progressCircle->setPercentage(0);

    circleLayout->addStretch();
    circleLayout->addWidget(progressCircle);
    circleLayout->addStretch();
    attendanceLayout->addLayout(circleLayout);

    // Right Side
    QVBoxLayout *statsLayout = new QVBoxLayout();
    statsLayout->setContentsMargins(15,0,0,0);
    statsLayout->setSpacing(12);

    QLabel *attendanceTitle = new QLabel("Today's Attendance");
    attendanceTitle->setMinimumWidth(260);
    attendanceTitle->setMinimumHeight(40);
    attendanceTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    attendanceTitle->setFont(FontManager::headingFont(18));
    attendanceTitle->setStyleSheet(QString("border:none; color:%1;").arg(Theme::Primary));
    statsLayout->addWidget(attendanceTitle);

    statsLayout->addSpacing(15);

    presentCount = new QLabel("🟢 - Present");
    presentCount->setStyleSheet(
        QString("color:%1; font-size:16px; border:none;").arg(Theme::Success));

    lateCount = new QLabel("🟡 - Late");
    lateCount->setStyleSheet(
        QString("color:%1; font-size:16px; border:none;").arg(Theme::Warning));

    absentCount = new QLabel("🔴 - Absent");
    absentCount->setStyleSheet(
        QString("color:%1; font-size:16px; border:none;").arg(Theme::Danger));

    statsLayout->addWidget(presentCount);
    statsLayout->addWidget(lateCount);
    statsLayout->addWidget(absentCount);
    statsLayout->addStretch();

    attendanceLayout->addLayout(statsLayout);

    // Week Card
    QFrame *weekCard = new QFrame();
    weekCard->setMinimumSize(550,200);
    weekCard->setStyleSheet(QString(R"(
QFrame{
    background:%1;
    border:1px solid %2;
    border-radius:20px;
}
)")
                                .arg(Theme::Surface)
                                .arg(Theme::Border));

    auto shadow2 = new QGraphicsDropShadowEffect;
    shadow2->setBlurRadius(25);
    shadow2->setOffset(0,4);
    shadow2->setColor(QColor(0,0,0,140));
    weekCard->setGraphicsEffect(shadow2);

    QVBoxLayout *weekLayout = new QVBoxLayout(weekCard);
    weekLayout->setContentsMargins(25,20,25,20);
    weekLayout->setSpacing(20);

    QLabel *weekTitle = new QLabel("This Week");
    weekTitle->setFont(FontManager::headingFont(18));
    weekTitle->setStyleSheet(QString("color:%1;").arg(Theme::Primary));
    weekLayout->addWidget(weekTitle);

    // Compute weekly data from DB
    Database db(std::string(PROJECT_SOURCE_DIR) + "/smart_attendance.db");
    db.initializeTables();
    auto allRecords = db.getAllRecords();
    auto allStudents = db.getAllStudents();
    int totalStudents = static_cast<int>(allStudents.size());

    // Count attendance per day of current week
    QDate today = QDate::currentDate();
    QDate monday = today.addDays(-(today.dayOfWeek() - 1));
    std::map<int, int> dayCount; // 1=Mon..7=Sun
    for (auto &r : allRecords) {
        QDate d = QDate::fromString(QString::fromStdString(r.date), "yyyy-MM-dd");
        if (d.isValid() && d >= monday && d <= today) {
            dayCount[d.dayOfWeek()]++;
        }
    }

    QStringList days = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
    QList<int> values;
    for (int i = 1; i <= 7; ++i) {
        int pct = (totalStudents > 0)
            ? (dayCount[i] * 100) / totalStudents
            : 0;
        values.append(pct);
    }

    QHBoxLayout *barsLayout = new QHBoxLayout();
    barsLayout->setSpacing(18);
    barsLayout->setAlignment(Qt::AlignBottom);

    for (int i = 0; i < 7; i++) {
        QVBoxLayout *dayLayout = new QVBoxLayout();

        QProgressBar *bar = new QProgressBar();
        bar->setOrientation(Qt::Vertical);
        bar->setRange(0,100);
        bar->setValue(values[i]);
        bar->setTextVisible(false);
        bar->setFixedSize(22,90);

        bar->setStyleSheet(QString(R"(
QProgressBar{
    border:1px solid %1;
    border-radius:8px;
    background:%2;
}
QProgressBar::chunk{
    background:%3;
    border-radius:8px;
}
)")
                               .arg(Theme::Border)
                               .arg(Theme::Input)
                               .arg(Theme::Gold));

        QLabel *day = new QLabel(days[i]);
        day->setAlignment(Qt::AlignCenter);
        day->setStyleSheet(QString("color:%1;").arg(Theme::Secondary));

        dayLayout->addWidget(bar,0,Qt::AlignCenter);
        dayLayout->addWidget(day);
        barsLayout->addLayout(dayLayout);
    }

    weekLayout->addLayout(barsLayout);
    topCards->addWidget(attendanceCard);
    topCards->addWidget(weekCard);
    mainLayout->addLayout(topCards);

    // Recent Check-ins
    recentCard = new QFrame();
    recentCard->setMinimumHeight(220);
    recentCard->setStyleSheet(QString(R"(
QFrame{
    background:%1;
    border:1px solid %2;
    border-radius:20px;
}
)")
                                  .arg(Theme::Surface)
                                  .arg(Theme::Border));

    auto *shadow3 = new QGraphicsDropShadowEffect;
    shadow3->setBlurRadius(25);
    shadow3->setOffset(0,4);
    shadow3->setColor(QColor(0,0,0,140));
    recentCard->setGraphicsEffect(shadow3);

    // Placeholder layout - will be rebuilt in refreshDashboard()
    new QVBoxLayout(recentCard);
    mainLayout->addWidget(recentCard);

    // Bottom Cards
    QHBoxLayout *bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(20);

    // Manage Students Card
    QFrame *manageCard = new QFrame();
    manageCard->setObjectName("manageCard");
    manageCard->setMinimumSize(95,95);
    manageCard->setStyleSheet(QString(R"(
#manageCard{
    background:%1;
    border:1px solid %2;
    border-radius:18px;
}
)")
                                  .arg(Theme::Surface)
                                  .arg(Theme::Border));

    auto *manageShadow = new QGraphicsDropShadowEffect;
    manageShadow->setBlurRadius(20);
    manageShadow->setOffset(0,4);
    manageShadow->setColor(QColor(0,0,0,120));
    manageCard->setGraphicsEffect(manageShadow);

    QVBoxLayout *manageLayout = new QVBoxLayout(manageCard);
    manageLayout->setContentsMargins(10,10,10,10);
    manageLayout->setSpacing(8);

    QLabel *manageIcon = new QLabel();
    manageIcon->setFixedSize(36,36);
    manageIcon->setAlignment(Qt::AlignCenter);
    manageIcon->setStyleSheet(QString(R"(
QLabel{
    background:%1;
    border-radius:18px;
    font-size:18px;
}
)")
                                  .arg(Theme::Gold));

    QLabel *manageTitle = new QLabel("Manage Students");
    manageTitle->setAlignment(Qt::AlignCenter);
    manageTitle->setFont(FontManager::headingFont(16));
    manageTitle->setStyleSheet(QString("color:%1;").arg(Theme::Primary));

    QLabel *manageDesc = new QLabel("View, edit and delete registered students.");
    manageDesc->setAlignment(Qt::AlignCenter);
    manageDesc->setWordWrap(true);
    manageDesc->setStyleSheet(
        QString("color:%1; font-size:16px;").arg(Theme::Secondary));

    QPushButton *manageButton = new QPushButton("Open");
    connect(manageButton, &QPushButton::clicked, this, [this]() {
        auto *window = new ManageStudentsWindow();
        window->show();
        this->close();
    });
    manageButton->setMinimumHeight(20);
    manageButton->setStyleSheet(QString(R"(
QPushButton{
    background:%1;
    color:%2;
    border:none;
    border-radius:10px;
    font-weight:bold;
}
QPushButton:hover{
    background:%3;
}
)")
                                    .arg(Theme::Gold)
                                    .arg(Theme::Card)
                                    .arg(Theme::Warning));

    manageLayout->addWidget(manageIcon);
    manageLayout->addWidget(manageTitle);
    manageLayout->addWidget(manageDesc);
    manageLayout->addStretch();
    manageLayout->addWidget(manageButton);
    bottomRow->addWidget(manageCard);

    // Attendance Records Card
    QFrame *recordCard = new QFrame();
    recordCard->setObjectName("recordCard");
    recordCard->setMinimumSize(95,95);
    recordCard->setStyleSheet(QString(R"(
#recordCard{
    background:%1;
    border:1px solid %2;
    border-radius:18px;
}
)")
                                  .arg(Theme::Surface)
                                  .arg(Theme::Border));

    auto *recordShadow = new QGraphicsDropShadowEffect;
    recordShadow->setBlurRadius(20);
    recordShadow->setOffset(0,4);
    recordShadow->setColor(QColor(0,0,0,120));
    recordCard->setGraphicsEffect(recordShadow);

    QVBoxLayout *recordLayout = new QVBoxLayout(recordCard);
    recordLayout->setContentsMargins(10,10,10,10);
    recordLayout->setSpacing(8);

    QLabel *recordIcon = new QLabel();
    recordIcon->setFixedSize(36,36);
    recordIcon->setAlignment(Qt::AlignCenter);
    recordIcon->setStyleSheet(QString(R"(
QLabel{
    background:%1;
    border-radius:18px;
    font-size:18px;
}
)")
                                  .arg(Theme::Warning));

    QLabel *recordTitle = new QLabel("Attendance Records");
    recordTitle->setAlignment(Qt::AlignCenter);
    recordTitle->setFont(FontManager::headingFont(16));
    recordTitle->setStyleSheet(QString("color:%1;").arg(Theme::Primary));

    QLabel *recordDesc = new QLabel("View daily, weekly and monthly attendance.");
    recordDesc->setAlignment(Qt::AlignCenter);
    recordDesc->setWordWrap(true);
    recordDesc->setStyleSheet(
        QString("color:%1; font-size:16px;").arg(Theme::Secondary));

    QPushButton *recordButton = new QPushButton("Open");
    connect(recordButton, &QPushButton::clicked, this, [this]() {
        auto *window = new AttendanceRecordsWindow();
        window->show();
        this->close();
    });
    recordButton->setMinimumHeight(20);
    recordButton->setStyleSheet(QString(R"(
QPushButton{
    background:%1;
    color:%2;
    border:none;
    border-radius:10px;
    font-weight:bold;
}
QPushButton:hover{
    background:%3;
}
)")
                                    .arg(Theme::Gold)
                                    .arg(Theme::Card)
                                    .arg(Theme::Warning));

    recordLayout->addWidget(recordIcon);
    recordLayout->addWidget(recordTitle);
    recordLayout->addWidget(recordDesc);
    recordLayout->addStretch();
    recordLayout->addWidget(recordButton);
    bottomRow->addWidget(recordCard);

    // Export Reports Card
    QFrame *exportCard = new QFrame();
    exportCard->setObjectName("exportCard");
    exportCard->setMinimumSize(95,95);
    exportCard->setStyleSheet(QString(R"(
#exportCard{
    background:%1;
    border:1px solid %2;
    border-radius:18px;
}
)")
                                  .arg(Theme::Surface)
                                  .arg(Theme::Border));

    auto *exportShadow = new QGraphicsDropShadowEffect;
    exportShadow->setBlurRadius(20);
    exportShadow->setOffset(0,4);
    exportShadow->setColor(QColor(0,0,0,120));
    exportCard->setGraphicsEffect(exportShadow);

    QVBoxLayout *exportLayout = new QVBoxLayout(exportCard);
    exportLayout->setContentsMargins(10,10,10,10);
    exportLayout->setSpacing(8);

    QLabel *exportIcon = new QLabel();
    exportIcon->setFixedSize(36,36);
    exportIcon->setAlignment(Qt::AlignCenter);
    exportIcon->setStyleSheet(QString(R"(
QLabel{
    background:%1;
    border-radius:18px;
    font-size:18px;
}
)")
                                  .arg(Theme::Success));

    QLabel *exportTitle = new QLabel("Export Reports");
    exportTitle->setAlignment(Qt::AlignCenter);
    exportTitle->setFont(FontManager::headingFont(16));
    exportTitle->setStyleSheet(QString("color:%1;").arg(Theme::Primary));

    QLabel *exportDesc = new QLabel("Export attendance reports as CSV.");
    exportDesc->setAlignment(Qt::AlignCenter);
    exportDesc->setWordWrap(true);
    exportDesc->setStyleSheet(
        QString("color:%1; font-size:16px;").arg(Theme::Secondary));

    QPushButton *exportButton = new QPushButton("Export");
    exportButton->setMinimumHeight(20);
    exportButton->setStyleSheet(QString(R"(
QPushButton{
    background:%1;
    color:%2;
    border:none;
    border-radius:10px;
    font-weight:bold;
}
QPushButton:hover{
    background:%3;
}
)")
                                    .arg(Theme::Gold)
                                    .arg(Theme::Card)
                                    .arg(Theme::Warning));

    connect(exportButton, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getSaveFileName(
            this, "Export Attendance CSV", "attendance_export.csv", "CSV Files (*.csv)");
        if (filePath.isEmpty()) return;

        Database db(std::string(PROJECT_SOURCE_DIR) + "/smart_attendance.db");
        db.initializeTables();
        auto records = db.getAllRecords();

        std::ofstream csv(filePath.toStdString());
        csv << "ID,Student Name,Roll No,Date,Time\n";
        for (auto &r : records) {
            csv << r.id << ","
                << r.studentName << ","
                << r.rollNumber << ","
                << r.date << ","
                << r.time << "\n";
        }
        csv.close();

        QMessageBox::information(this, "Export Complete",
                                 "Attendance data exported to:\n" + filePath);
    });

    exportLayout->addWidget(exportIcon);
    exportLayout->addWidget(exportTitle);
    exportLayout->addWidget(exportDesc);
    exportLayout->addStretch();
    exportLayout->addWidget(exportButton);
    bottomRow->addWidget(exportCard);


    // =========================================================
    // Manage Subjects Card
    // =========================================================

    QFrame *subjectCard = new QFrame();
    subjectCard->setObjectName("subjectCard");
    subjectCard->setMinimumSize(95,95);

    subjectCard->setStyleSheet(QString(R"(
#subjectCard{
    background:%1;
    border:1px solid %2;
    border-radius:18px;
}
)")
                                   .arg(Theme::Surface)
                                   .arg(Theme::Border));

    auto *subjectShadow = new QGraphicsDropShadowEffect;
    subjectShadow->setBlurRadius(20);
    subjectShadow->setOffset(0,4);
    subjectShadow->setColor(QColor(0,0,0,120));
    subjectCard->setGraphicsEffect(subjectShadow);

    QVBoxLayout *subjectLayout = new QVBoxLayout(subjectCard);
    subjectLayout->setContentsMargins(10,10,10,10);
    subjectLayout->setSpacing(8);


    // Icon
    QLabel *subjectIcon = new QLabel();
    subjectIcon->setFixedSize(36,36);
    subjectIcon->setAlignment(Qt::AlignCenter);

    subjectIcon->setStyleSheet(QString(R"(
QLabel{
    background:%1;
    border-radius:18px;
    font-size:18px;
}
)")
                                   .arg(Theme::Gold));


    // Title
    QLabel *subjectTitle = new QLabel("Manage Subjects");
    subjectTitle->setAlignment(Qt::AlignCenter);
    subjectTitle->setFont(FontManager::headingFont(16));

    subjectTitle->setStyleSheet(
        QString("color:%1;").arg(Theme::Primary));


    // Description
    QLabel *subjectDesc = new QLabel(
        "Add, edit and manage subjects."
        );

    subjectDesc->setAlignment(Qt::AlignCenter);
    subjectDesc->setWordWrap(true);

    subjectDesc->setStyleSheet(
        QString("color:%1; font-size:16px;")
            .arg(Theme::Secondary));


    // Open Button
    QPushButton *subjectButton = new QPushButton("Open");
    subjectButton->setMinimumHeight(20);

    subjectButton->setStyleSheet(QString(R"(
QPushButton{
    background:%1;
    color:%2;
    border:none;
    border-radius:10px;
    font-weight:bold;
}
QPushButton:hover{
    background:%3;
}
)")
                                     .arg(Theme::Gold)
                                     .arg(Theme::Card)
                                     .arg(Theme::Warning));


    // Open Subject Management Window
    connect(subjectButton, &QPushButton::clicked, this, [this]() {

        auto *window = new SubjectManagementWindow();

        window->show();

        this->close();
    });


    // Add widgets to card
    subjectLayout->addWidget(subjectIcon);
    subjectLayout->addWidget(subjectTitle);
    subjectLayout->addWidget(subjectDesc);
    subjectLayout->addStretch();
    subjectLayout->addWidget(subjectButton);


    // Add card to bottom row
    bottomRow->addWidget(subjectCard);
    // Schedule Editor Card
    QFrame *scheduleCard = new QFrame();
    scheduleCard->setObjectName("scheduleCard");
    scheduleCard->setMinimumSize(95, 95);

    scheduleCard->setStyleSheet(QString(R"(
#scheduleCard{
    background:%1;
    border:1px solid %2;
    border-radius:18px;
}
)")
                                    .arg(Theme::Surface)
                                    .arg(Theme::Border));

    auto *scheduleShadow = new QGraphicsDropShadowEffect;
    scheduleShadow->setBlurRadius(20);
    scheduleShadow->setOffset(0,4);
    scheduleShadow->setColor(QColor(0,0,0,120));

    scheduleCard->setGraphicsEffect(scheduleShadow);


    // Schedule Card Layout
    QVBoxLayout *scheduleLayout =
        new QVBoxLayout(scheduleCard);

    scheduleLayout->setContentsMargins(10,10,10,10);
    scheduleLayout->setSpacing(8);


    // Icon
    QLabel *scheduleIcon = new QLabel();

    scheduleIcon->setFixedSize(36,36);
    scheduleIcon->setAlignment(Qt::AlignCenter);

    scheduleIcon->setStyleSheet(QString(R"(
QLabel{
    background:%1;
    border-radius:18px;
    font-size:18px;
}
)")
                                    .arg(Theme::Warning));


    // Title
    QLabel *scheduleTitle =
        new QLabel("Schedule Editor");

    scheduleTitle->setAlignment(Qt::AlignCenter);

    scheduleTitle->setFont(
        FontManager::headingFont(16)
        );

    scheduleTitle->setStyleSheet(
        QString("color:%1;")
            .arg(Theme::Primary)
        );


    // Description
    QLabel *scheduleDesc =
        new QLabel(
            "Create and manage weekly class schedules."
            );

    scheduleDesc->setAlignment(Qt::AlignCenter);
    scheduleDesc->setWordWrap(true);

    scheduleDesc->setStyleSheet(
        QString("color:%1; font-size:16px;")
            .arg(Theme::Secondary)
        );


    // Open Button
    QPushButton *scheduleButton =
        new QPushButton("Open");

    scheduleButton->setMinimumHeight(20);

    scheduleButton->setStyleSheet(QString(R"(
QPushButton{
    background:%1;
    color:%2;
    border:none;
    border-radius:10px;
    font-weight:bold;
}
QPushButton:hover{
    background:%3;
}
)")
                                      .arg(Theme::Gold)
                                      .arg(Theme::Card)
                                      .arg(Theme::Warning));


    connect(scheduleButton, &QPushButton::clicked, this, [this]() {

        auto *window = new ScheduleEditor();
        window->show();

        this->close();

    });


    // Add Widgets
    scheduleLayout->addWidget(scheduleIcon);
    scheduleLayout->addWidget(scheduleTitle);
    scheduleLayout->addWidget(scheduleDesc);

    scheduleLayout->addStretch();

    scheduleLayout->addWidget(scheduleButton);


    // Add Card to Bottom Row
    bottomRow->addWidget(scheduleCard);


    // Add bottom row to main layout
    mainLayout->addLayout(bottomRow);
    setLayout(mainLayout);
}
