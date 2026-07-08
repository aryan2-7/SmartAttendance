#include "AdminDashboard.h"
#include "../auth/FontManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QDate>
#include <QGraphicsDropShadowEffect>
#include <QProgressBar>


AdminDashboard::AdminDashboard(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void AdminDashboard::setupUI()
{
    setWindowTitle("Admin Dashboard");
    resize(1400,850);

    setStyleSheet(R"(
QWidget{
    background:#0F172A;
    color:white;
}

QLineEdit{
    background:#111827;
    border:1px solid #334155;
    border-radius:12px;
    padding:10px;
    color:white;
    font-size:14px;
}

QLineEdit:focus{
    border:1px solid #38BDF8;
}

QPushButton{
    background:#1E293B;
    color:white;
    border:1px solid #334155;
    border-radius:10px;
    padding:10px 20px;
}

QPushButton:hover{
    background:#334155;
}
)");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30,25,30,25);
    mainLayout->setSpacing(25);

    //----------------------------------------------------
    // Header
    //----------------------------------------------------

    QHBoxLayout *header = new QHBoxLayout();

    QPushButton *backButton = new QPushButton("← Back");
    backButton->setFixedSize(105,38);

    QVBoxLayout *titleLayout = new QVBoxLayout();

    QLabel *title = new QLabel("Welcome, Administrator");
    title->setFont(FontManager::headingFont(24));

    QLabel *date = new QLabel(
        QDate::currentDate().toString("dddd • MMMM d, yyyy")
        );

    date->setStyleSheet("color:#94A3B8;");

    titleLayout->addWidget(title);
    titleLayout->addWidget(date);

    QLineEdit *search = new QLineEdit();
    search->setPlaceholderText("🔍  Search students...");
    search->setFixedWidth(280);
    search->setFixedHeight(42);

    header->addWidget(backButton);
    header->addSpacing(25);
    header->addLayout(titleLayout);
    header->addStretch();
    header->addWidget(search);

    mainLayout->addLayout(header);

    //----------------------------------------------------
    // Dashboard Row
    //----------------------------------------------------

    QHBoxLayout *topCards = new QHBoxLayout();
    topCards->setSpacing(20);

    QFrame *attendanceCard = new QFrame();
    attendanceCard->setMinimumSize(400,200);



    attendanceCard->setStyleSheet(R"(
QFrame{
    background:#111827;
    border:1px solid #334155;
    border-radius:20px;
}
)");

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
    circle->setFixedSize(130,130);

    circle->setStyleSheet(R"(
QFrame{
    background:#111827;
    border:10px solid #38BDF8;
    border-radius:70px;
}
)");

    QVBoxLayout *circleTextLayout = new QVBoxLayout(circle);

    QLabel *percent = new QLabel("-%");
    percent->setAlignment(Qt::AlignCenter);
    percent->setFont(FontManager::headingFont(24));
    percent->setStyleSheet("color:white; border:none;");

    QLabel *presentLabel = new QLabel("Present");
    presentLabel->setAlignment(Qt::AlignCenter);
    presentLabel->setStyleSheet(
        "color:#94A3B8;"
        "border:none;"
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

    QLabel *attendanceTitle = new QLabel("Today's Attendance");
    attendanceTitle->setFont(FontManager::headingFont(18));
    attendanceTitle->setStyleSheet("border:none;");
    statsLayout->addWidget(attendanceTitle);

    statsLayout->addSpacing(15);

    QLabel *present = new QLabel("🟢 - Present");
    present->setStyleSheet(
        "color:#4ADE80;"
        "font-size:16px;"
        "border:none;"
        );

    QLabel *late = new QLabel("🟡 - Late");
    late->setStyleSheet(
        "color:#FBBF24;"
        "font-size:16px;"
        "border:none;"
        );

    QLabel *absent = new QLabel("🔴 - Absent");
    absent->setStyleSheet(
        "color:#F87171;"
        "font-size:16px;"
        "border:none;"
        );

    statsLayout->addWidget(present);
    statsLayout->addWidget(late);
    statsLayout->addWidget(absent);

    statsLayout->addStretch();

    attendanceLayout->addLayout(statsLayout);
//this week

    QFrame *weekCard = new QFrame();
    weekCard->setMinimumSize(550,200);


    weekCard->setStyleSheet(R"(
QFrame{
    background:#111827;
    border:1px solid #334155;
    border-radius:20px;
}
)");

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

        bar->setStyleSheet(R"(
QProgressBar{
    border:1px solid #334155;
    border-radius:8px;
    background:#1E293B;
}

QProgressBar::chunk{
    background:#38BDF8;
    border-radius:8px;
}
)");

        QLabel *day = new QLabel(days[i]);
        day->setAlignment(Qt::AlignCenter);
        day->setStyleSheet("color:#94A3B8;");

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

    recentCard->setStyleSheet(R"(
QFrame{
    background:#111827;
    border:1px solid #334155;
    border-radius:20px;
}
)");

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

        row->setStyleSheet(R"(
QFrame{
    background:#1E293B;
    border-radius:12px;
}
)");

        row->setMinimumHeight(50);

        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(20,10,20,10);

        QLabel *avatar = new QLabel("👤");
        avatar->setStyleSheet("font-size:24px;");

        QLabel *name = new QLabel(students[i]);
        name->setFont(FontManager::buttonFont(14));

        QLabel *badge = new QLabel(status[i]);

        if(status[i]=="Present")
            badge->setStyleSheet(
                "background:#14532D;"
                "color:#4ADE80;"
                "padding:6px 14px;"
                "border-radius:10px;"
                );

        else if(status[i]=="Late")
            badge->setStyleSheet(
                "background:#78350F;"
                "color:#FACC15;"
                "padding:6px 14px;"
                "border-radius:10px;"
                );

        else
            badge->setStyleSheet(
                "background:#7F1D1D;"
                "color:#F87171;"
                "padding:6px 14px;"
                "border-radius:10px;"
                );

        QLabel *timeLabel = new QLabel(time[i]);
        timeLabel->setStyleSheet("color:#94A3B8;");

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

manageCard->setStyleSheet(R"(
#manageCard{
    background:#111827;
    border:1px solid #334155;
    border-radius:18px;
}
)");

    auto *manageShadow = new QGraphicsDropShadowEffect;
    manageShadow->setBlurRadius(20);
    manageShadow->setOffset(0,4);
    manageShadow->setColor(QColor(0,0,0,120));
    manageCard->setGraphicsEffect(manageShadow);

    QVBoxLayout *manageLayout = new QVBoxLayout(manageCard);
    manageLayout->setContentsMargins(10,10,10,10);

    manageLayout->setSpacing(3);
    QLabel *manageIcon = new QLabel("👥");
    manageIcon->setAlignment(Qt::AlignCenter);
    manageIcon->setStyleSheet("font-size:18px;");

    QLabel *manageTitle = new QLabel("Manage Students");
    manageTitle->setAlignment(Qt::AlignCenter);
    manageTitle->setFont(FontManager::headingFont(16));

    QLabel *manageDesc = new QLabel(
        "View, edit and delete\nregistered students."
        );

    manageDesc->setAlignment(Qt::AlignCenter);
    manageDesc->setWordWrap(true);
    manageDesc->setStyleSheet(
    "color:#94A3B8;"
    "font-size:16px;"
);

    QPushButton *manageButton = new QPushButton("Open");
    manageButton->setMinimumHeight(32);

    manageButton->setStyleSheet(R"(
QPushButton{
    background:#2563EB;
    color:white;
    border:none;
    border-radius:10px;
    font-weight:bold;
}

QPushButton:hover{
    background:#1D4ED8;
}
)");

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

    recordCard->setStyleSheet(R"(
#recordCard{
    background:#111827;
    border:1px solid #334155;
    border-radius:18px;
}
)");

    auto *recordShadow = new QGraphicsDropShadowEffect;
    recordShadow->setBlurRadius(20);
    recordShadow->setOffset(0,4);
    recordShadow->setColor(QColor(0,0,0,120));
    recordCard->setGraphicsEffect(recordShadow);

    QVBoxLayout *recordLayout = new QVBoxLayout(recordCard);
    recordLayout->setContentsMargins(10,10,10,10);
    recordLayout->setSpacing(3);

    QLabel *recordIcon = new QLabel("📋");
    recordIcon->setAlignment(Qt::AlignCenter);
    recordIcon->setStyleSheet("font-size:18px;");

    QLabel *recordTitle = new QLabel("Attendance Records");
    recordTitle->setAlignment(Qt::AlignCenter);
    recordTitle->setFont(FontManager::headingFont(16));

    QLabel *recordDesc = new QLabel(
        "View daily, weekly and\nmonthly attendance."
        );

    recordDesc->setAlignment(Qt::AlignCenter);
    recordDesc->setWordWrap(true);
    recordDesc->setStyleSheet(
        "color:#94A3B8;"
        "font-size:16px;"
        );

    QPushButton *recordButton = new QPushButton("Open");
    recordButton->setMinimumHeight(32);

    recordButton->setStyleSheet(R"(
QPushButton{
    background:#2563EB;
    color:white;
    border:none;
    border-radius:10px;
    font-weight:bold;
}
QPushButton:hover{
    background:#1D4ED8;
}
)");

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

    exportCard->setStyleSheet(R"(
#exportCard{
    background:#111827;
    border:1px solid #334155;
    border-radius:18px;
}
)");

    auto *exportShadow = new QGraphicsDropShadowEffect;
    exportShadow->setBlurRadius(20);
    exportShadow->setOffset(0,4);
    exportShadow->setColor(QColor(0,0,0,120));
    exportCard->setGraphicsEffect(exportShadow);

    QVBoxLayout *exportLayout = new QVBoxLayout(exportCard);
    exportLayout->setContentsMargins(10,10,10,10);
    exportLayout->setSpacing(3);

    QLabel *exportIcon = new QLabel("📤");
    exportIcon->setAlignment(Qt::AlignCenter);
    exportIcon->setStyleSheet("font-size:18px;");

    QLabel *exportTitle = new QLabel("Export Reports");
    exportTitle->setAlignment(Qt::AlignCenter);
    exportTitle->setFont(FontManager::headingFont(16));

    QLabel *exportDesc = new QLabel(
        "Export attendance\nreports as PDF or CSV."
        );

    exportDesc->setAlignment(Qt::AlignCenter);
    exportDesc->setWordWrap(true);
    exportDesc->setStyleSheet(
        "color:#94A3B8;"
        "font-size:16px;"
        );

    QPushButton *exportButton = new QPushButton("Open");
    exportButton->setMinimumHeight(32);

    exportButton->setStyleSheet(R"(
QPushButton{
    background:#2563EB;
    color:white;
    border:none;
    border-radius:10px;
    font-weight:bold;
}
QPushButton:hover{
    background:#1D4ED8;
}
)");

    exportLayout->addWidget(exportIcon);
    exportLayout->addWidget(exportTitle);
    exportLayout->addWidget(exportDesc);
    exportLayout->addStretch();
    exportLayout->addWidget(exportButton);

    bottomRow->addWidget(exportCard);

    mainLayout->addLayout(bottomRow);

    setLayout(mainLayout);
    }