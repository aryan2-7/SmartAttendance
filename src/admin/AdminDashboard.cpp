#include "AdminDashboard.h"
#include "../auth/FontManager.h"
#include "../theme/Theme.h"
#include "ManageStudentsWindow.h"
#include "AttendanceRecordsWindow.h"
#include "AdministratorWindow.h"
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


AdminDashboard::AdminDashboard(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void AdminDashboard::setupUI()
{
    setWindowTitle("Admin Dashboard");
    resize(1400,850);
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

    //----------------------------------------------------
    // Header
    //----------------------------------------------------

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
        QDate::currentDate().toString("dddd • MMMM d, yyyy")
        );

    date->setStyleSheet(QString("color:%1;").arg(Theme::Secondary));

    titleLayout->addWidget(title);
    titleLayout->addWidget(date);


    header->addWidget(backButton);
    header->addSpacing(25);
    header->addLayout(titleLayout);
    header->addStretch();


    mainLayout->addLayout(header);

    //----------------------------------------------------
    // Dashboard Row
    //----------------------------------------------------

    QHBoxLayout *topCards = new QHBoxLayout();
    topCards->setSpacing(20);

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
    //================ Attendance Card Layout =================

    QHBoxLayout *attendanceLayout = new QHBoxLayout(attendanceCard);
    attendanceLayout->setContentsMargins(30,25,30,25);
    attendanceLayout->setSpacing(25);

    // Left Side (Circle)
    QVBoxLayout *circleLayout = new QVBoxLayout();

    QFrame *circle = new QFrame();
    circle->setFixedSize(120,120);

    circle->setStyleSheet(QString(R"(
QFrame{
    background:transparent;
    border:8px solid %1;
    border-radius:60px;
}
)")
                              .arg(Theme::Gold));

    QVBoxLayout *circleTextLayout = new QVBoxLayout(circle);

    QLabel *percent = new QLabel("-%");
    percent->setAlignment(Qt::AlignCenter);
    percent->setFont(FontManager::headingFont(24));
    percent->setStyleSheet(QString("color:%1; border:none;").arg(Theme::Primary));

    QLabel *presentLabel = new QLabel("Present");
    presentLabel->setAlignment(Qt::AlignCenter);
    presentLabel->setStyleSheet(
        QString("color:%1;"
                "border:none;").arg(Theme::Secondary)
        );

    circleTextLayout->addStretch();
    circleTextLayout->addWidget(percent);
    circleTextLayout->addWidget(presentLabel);
    circleTextLayout->addStretch();

    circleLayout->addStretch();
    circleLayout->addWidget(circle);
    circleLayout->addStretch();

    attendanceLayout->addLayout(circleLayout);

    // Right Side
    QVBoxLayout *statsLayout = new QVBoxLayout();
    statsLayout->setContentsMargins(15,0,0,0);
    statsLayout->setSpacing(12);

    QLabel *attendanceTitle = new QLabel("Today's Attendance");
    attendanceTitle->setMinimumWidth(260);
    attendanceTitle->setMinimumHeight(40);
    attendanceTitle->setSizePolicy(QSizePolicy::Expanding,
                                   QSizePolicy::Preferred);
    attendanceTitle->setFont(FontManager::headingFont(18));
    attendanceTitle->setStyleSheet(QString("border:none; color:%1;").arg(Theme::Primary));
    statsLayout->addWidget(attendanceTitle);

    statsLayout->addSpacing(15);

    QLabel *present = new QLabel("🟢 - Present");
    present->setStyleSheet(
        QString("color:%1;"
                "font-size:16px;"
                "border:none;").arg(Theme::Success)
        );

    QLabel *late = new QLabel("🟡 - Late");
    late->setStyleSheet(
        QString("color:%1;"
                "font-size:16px;"
                "border:none;").arg(Theme::Warning)
        );

    QLabel *absent = new QLabel("🔴 - Absent");
    absent->setStyleSheet(
        QString("color:%1;"
                "font-size:16px;"
                "border:none;").arg(Theme::Danger)
        );

    statsLayout->addWidget(present);
    statsLayout->addWidget(late);
    statsLayout->addWidget(absent);

    statsLayout->addStretch();

    attendanceLayout->addLayout(statsLayout);
    //this week

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


    QHBoxLayout *barsLayout = new QHBoxLayout();
    barsLayout->setSpacing(18);
    barsLayout->setAlignment(Qt::AlignBottom);
    QStringList days = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
    QList<int> values = {65,82,90,78,60,45,88};

    for(int i=0;i<7;i++)
    {
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


    //----------------------------------------------------
    // Recent Check-ins
    //----------------------------------------------------

    QFrame *recentCard = new QFrame();
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

    QVBoxLayout *recentLayout = new QVBoxLayout(recentCard);
    recentLayout->setContentsMargins(25,20,25,20);
    recentLayout->setSpacing(15);

    QLabel *recentTitle = new QLabel("Recent Check-ins");
    recentTitle->setFont(FontManager::headingFont(18));
    recentTitle->setStyleSheet(QString("color:%1;").arg(Theme::Primary));

    recentLayout->addWidget(recentTitle);

    QStringList students = {
        "Prachika",
        "Subhechha",
        "Aryan"
    };

    QStringList status = {
        "Present",
        "Late",
        "Present"
    };

    QStringList time = {
        "08:42 AM",
        "09:01 AM",
        "08:37 AM",
    };

    for(int i=0;i<students.size();i++)
    {
        QFrame *row = new QFrame();

        row->setStyleSheet(QString(R"(
QFrame{
    background:%1;
    border-radius:12px;
}
)")
                               .arg(Theme::Input));

        row->setMinimumHeight(50);

        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(20,10,20,10);

        QLabel *avatar = new QLabel("👤");
        avatar->setStyleSheet("font-size:24px;");
        QLabel *name = new QLabel(students[i]);
        name->setFont(FontManager::buttonFont(14));
        name->setStyleSheet(QString("color:%1;").arg(Theme::Primary));

        QLabel *badge = new QLabel(status[i]);

        if(status[i]=="Present")
            badge->setStyleSheet(
                QString("background:%1;"
                        "color:%2;"
                        "padding:6px 14px;"
                        "border-radius:10px;")
                    .arg(Theme::Card)
                    .arg(Theme::Success)
                );

        else if(status[i]=="Late")
            badge->setStyleSheet(
                QString("background:%1;"
                        "color:%2;"
                        "padding:6px 14px;"
                        "border-radius:10px;")
                    .arg(Theme::Card)
                    .arg(Theme::Warning)
                );

        else
            badge->setStyleSheet(
                QString("background:%1;"
                        "color:%2;"
                        "padding:6px 14px;"
                        "border-radius:10px;")
                    .arg(Theme::Card)
                    .arg(Theme::Danger)
                );

        QLabel *timeLabel = new QLabel(time[i]);
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
    mainLayout->addWidget(recentCard);
    //----------------------------------------------------
    // Bottom Cards Placeholder
    //----------------------------------------------------

    QHBoxLayout *bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(20);

    //================ Manage Students Card =================

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

    QLabel *manageDesc = new QLabel(
        "View, edit and delete\nregistered students."
        );

    manageDesc->setAlignment(Qt::AlignCenter);
    manageDesc->setWordWrap(true);
    manageDesc->setStyleSheet(
        QString("color:%1;"
                "font-size:16px;").arg(Theme::Secondary)
        );

    QPushButton *manageButton = new QPushButton("Open");
    connect(manageButton, &QPushButton::clicked, this, [this]()
            {
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

    //================ Attendance Records Card =================

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

    QLabel *recordDesc = new QLabel(
        "View daily, weekly and\nmonthly attendance."
        );

    recordDesc->setAlignment(Qt::AlignCenter);
    recordDesc->setWordWrap(true);
    recordDesc->setStyleSheet(
        QString("color:%1;"
                "font-size:16px;").arg(Theme::Secondary)
        );

    QPushButton *recordButton = new QPushButton("Open");
    connect(recordButton, &QPushButton::clicked, this, [this]()
            {
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

    //================ Export Reports Card =================

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

    QLabel *exportDesc = new QLabel(
        "Export attendance\nreports as PDF or CSV."
        );

    exportDesc->setAlignment(Qt::AlignCenter);
    exportDesc->setWordWrap(true);
    exportDesc->setStyleSheet(
        QString("color:%1;"
                "font-size:16px;").arg(Theme::Secondary)
        );

    QPushButton *exportButton = new QPushButton("Open");
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

    exportLayout->addWidget(exportIcon);
    exportLayout->addWidget(exportTitle);
    exportLayout->addWidget(exportDesc);
    exportLayout->addStretch();
    exportLayout->addWidget(exportButton);

    bottomRow->addWidget(exportCard);

    mainLayout->addLayout(bottomRow);

    setLayout(mainLayout);
}