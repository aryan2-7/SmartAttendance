#include "AttendanceRecordsWindow.h"
#include "../auth/FontManager.h"
#include "AdminDashboard.h"
#include "../theme/Theme.h"
#include "../db/db.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>

#include <QDate>
#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <QColor>
#include <set>
#include <map>

AttendanceRecordsWindow::AttendanceRecordsWindow(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setupUI();
}

void AttendanceRecordsWindow::setupUI()
{
    setWindowTitle("Attendance Records");
    resize(1400,850);
    setObjectName("AttendanceRecordsWindow");

    setStyleSheet(QString(R"(
QWidget#AttendanceRecordsWindow{
    background:%1;
}
QFrame#card{
    background:%3;
    border:1px solid %2;
    border-radius:16px;
}
QTableWidget{
    background:%6;
    border:1px solid %2;
    border-radius:14px;
    gridline-color:%2;
    color:%4;
    font-size:14px;
    selection-background-color:%7;
}
QHeaderView::section{
    background:%3;
    color:%4;
    border:none;
    padding:10px;
    font-weight:bold;
}
QTableWidget::item{
    background:%6;
    color:%4;
}
QPushButton{
    background:%3;
    color:%4;
    border:1px solid %2;
    border-radius:10px;
    padding:10px 18px;
}
QPushButton:hover{
    background:%5;
}
QLineEdit, QDateEdit{
    background:%6;
    color:%4;
    border:1px solid %2;
    border-radius:10px;
    padding:10px;
    font-size:14px;
}
QLineEdit:focus, QDateEdit:focus{
    border:1px solid %7;
}
QLabel{
    border:none;
    background:transparent;
    color:%4;
}
)")
                      .arg(Theme::Card)
                      .arg(Theme::Border)
                      .arg(Theme::Surface)
                      .arg(Theme::Primary)
                      .arg(Theme::Hover)
                      .arg(Theme::Input)
                      .arg(Theme::Gold));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30,25,30,25);
    mainLayout->setSpacing(25);
    mainLayout->setAlignment(Qt::AlignTop);

    // Header
    QHBoxLayout *header = new QHBoxLayout();
    QPushButton *backButton = new QPushButton("Back");
    connect(backButton, &QPushButton::clicked, this, [this]()
            {
                auto *window = new AdminDashboard();
                window->show();
                this->close();
            });
    backButton->setFixedSize(100,40);

    QLabel *title = new QLabel("Attendance Records");
    title->setFont(FontManager::headingFont(26));
    title->setStyleSheet(QString("color:%1;").arg(Theme::Gold));

    header->addWidget(backButton);
    header->addSpacing(20);
    header->addWidget(title);
    header->addStretch();
    mainLayout->addLayout(header);

    // Card
    QFrame *card = new QFrame();
    card->setObjectName("card");
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20,20,20,20);
    cardLayout->setSpacing(20);

    QLabel *rosterTitle = new QLabel("Today's Attendance Roster");
    rosterTitle->setFont(FontManager::headingFont(18));
    rosterTitle->setStyleSheet(QString("color:%1;").arg(Theme::Gold));
    cardLayout->addWidget(rosterTitle);

    // Fetch data
    Database db(std::string(PROJECT_SOURCE_DIR) + "/smart_attendance.db");
    db.initializeTables();
    auto students = db.getAllStudents();
    auto records = db.getAllRecords();

    std::string today = QDate::currentDate().toString("yyyy-MM-dd").toStdString();
    std::set<int> presentRolls;
    std::map<int, std::string> presentTimes;
    for (auto &r : records) {
        if (r.date == today) {
            presentRolls.insert(r.rollNumber);
            if (presentTimes.count(r.rollNumber) == 0)
                presentTimes[r.rollNumber] = r.time;
        }
    }

    QTableWidget *rosterTable = new QTableWidget();
    rosterTable->setColumnCount(3);
    rosterTable->setHorizontalHeaderLabels({"Student Name", "Roll No", "Status"});
    rosterTable->horizontalHeader()->setStretchLastSection(true);
    rosterTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    rosterTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    rosterTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    rosterTable->setSelectionMode(QAbstractItemView::NoSelection);
    rosterTable->verticalHeader()->hide();
    rosterTable->setShowGrid(false);
    rosterTable->verticalHeader()->setDefaultSectionSize(40);

    rosterTable->setShowGrid(true);
    rosterTable->setStyleSheet(QString(R"(
        QTableWidget{
            background:%1;
            border:1px solid %2;
            border-radius:14px;
            gridline-color:%2;
            selection-background-color:%3;
            color:%4;
            font-size:14px;
        }
        QHeaderView::section{
            background:%5;
            color:%4;
            border:none;
            padding:10px;
            font-weight:bold;
        }
    )")
        .arg(Theme::Input)
        .arg(Theme::Border)
        .arg(Theme::Gold)
        .arg(Theme::Primary)
        .arg(Theme::Surface));

    rosterTable->setRowCount(static_cast<int>(students.size()));
    for (size_t i = 0; i < students.size(); ++i) {
        auto &s = students[i];

        QTableWidgetItem *nameItem = new QTableWidgetItem(QString::fromStdString(s.name));
        nameItem->setForeground(QColor(Theme::Primary));
        rosterTable->setItem(i, 0, nameItem);

        QTableWidgetItem *rollItem = new QTableWidgetItem(QString::number(s.rollNumber));
        rollItem->setForeground(QColor(Theme::Secondary));
        rosterTable->setItem(i, 1, rollItem);

        bool isPresent = presentRolls.count(s.rollNumber) > 0;
        bool isLate = false;
        if (isPresent) {
            std::string t = presentTimes[s.rollNumber];
            isLate = (t > "09:00:00");
        }

        QString statusText;
        QColor statusColor;
        if (isPresent && isLate) {
            statusText = "Late";
            statusColor = QColor(Theme::Warning);
        } else if (isPresent) {
            statusText = "Present";
            statusColor = QColor(Theme::Success);
        } else {
            statusText = "Absent";
            statusColor = QColor(Theme::Danger);
        }

        QTableWidgetItem *statusItem = new QTableWidgetItem(statusText);
        statusItem->setForeground(statusColor);
        statusItem->setFont(FontManager::buttonFont(14));
        rosterTable->setItem(i, 2, statusItem);
    }

    cardLayout->addWidget(rosterTable);
    mainLayout->addWidget(card);

    setLayout(mainLayout);
}
