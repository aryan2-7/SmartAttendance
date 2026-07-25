#include "AttendanceRecordsWindow.h"
#include "../auth/FontManager.h"
#include "AdminDashboard.h"
#include "../theme/Theme.h"
#include "../db/Database.h"
#include "../db/StudentDAO.h"
#include "../db/SubjectDAO.h"
#include "../db/AttendanceDAO.h"
#include "../db/DbPath.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>

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
    refreshView();
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
QLineEdit, QDateEdit, QComboBox{
    background:%6;
    color:%4;
    border:1px solid %2;
    border-radius:10px;
    padding:10px;
    font-size:14px;
}
QLineEdit:focus, QDateEdit:focus, QComboBox:focus{
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
    mainLayout->setSpacing(20);
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

    // Filters
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(15);

    QLabel *subjectLabel = new QLabel("Subject:");
    subjectCombo = new QComboBox();
    subjectCombo->setMinimumWidth(240);

    QLabel *rangeLabel = new QLabel("Range:");
    rangeCombo = new QComboBox();
    rangeCombo->addItems({"Today", "This Week", "This Month", "All Time"});

    QPushButton *refreshButton = new QPushButton("Refresh");
    refreshButton->setFixedSize(110,42);

    filterLayout->addWidget(subjectLabel);
    filterLayout->addWidget(subjectCombo);
    filterLayout->addSpacing(15);
    filterLayout->addWidget(rangeLabel);
    filterLayout->addWidget(rangeCombo);
    filterLayout->addStretch();
    filterLayout->addWidget(refreshButton);
    mainLayout->addLayout(filterLayout);

    connect(subjectCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AttendanceRecordsWindow::refreshView);
    connect(rangeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AttendanceRecordsWindow::refreshView);
    connect(refreshButton, &QPushButton::clicked, this, &AttendanceRecordsWindow::refreshView);

    rangeSummaryLabel = new QLabel();
    rangeSummaryLabel->setStyleSheet(QString("color:%1; font-size:13px;").arg(Theme::Secondary));
    mainLayout->addWidget(rangeSummaryLabel);

    // Roster card
    QFrame *card = new QFrame();
    card->setObjectName("card");
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20,20,20,20);
    cardLayout->setSpacing(20);

    QLabel *rosterTitle = new QLabel("Attendance Log");
    rosterTitle->setFont(FontManager::headingFont(18));
    rosterTitle->setStyleSheet(QString("color:%1;").arg(Theme::Gold));
    cardLayout->addWidget(rosterTitle);

    rosterTable = new QTableWidget();
    rosterTable->setColumnCount(4);
    rosterTable->setHorizontalHeaderLabels({"Student Name", "Roll No", "Date", "Time"});
    rosterTable->horizontalHeader()->setStretchLastSection(true);
    rosterTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    rosterTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    rosterTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    rosterTable->setSelectionMode(QAbstractItemView::NoSelection);
    rosterTable->verticalHeader()->hide();
    rosterTable->setMinimumHeight(260);
    cardLayout->addWidget(rosterTable);
    mainLayout->addWidget(card);

    // Attendance % card (per-subject only)
    QFrame *percentCard = new QFrame();
    percentCard->setObjectName("card");
    QVBoxLayout *percentLayout = new QVBoxLayout(percentCard);
    percentLayout->setContentsMargins(20,20,20,20);
    percentLayout->setSpacing(20);

    QLabel *percentTitle = new QLabel("Attendance % (select a subject to view)");
    percentTitle->setFont(FontManager::headingFont(18));
    percentTitle->setStyleSheet(QString("color:%1;").arg(Theme::Gold));
    percentLayout->addWidget(percentTitle);

    percentTable = new QTableWidget();
    percentTable->setColumnCount(4);
    percentTable->setHorizontalHeaderLabels({"Student Name", "Roll No", "Attendance %", "Status"});
    percentTable->horizontalHeader()->setStretchLastSection(true);
    percentTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    percentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    percentTable->setSelectionMode(QAbstractItemView::NoSelection);
    percentTable->verticalHeader()->hide();
    percentTable->setMinimumHeight(220);
    percentLayout->addWidget(percentTable);
    mainLayout->addWidget(percentCard);

    setLayout(mainLayout);

    // Populate subject filter
    Database db(appDbPath());
    db.initializeTables();
    SubjectDAO subjectDAO(db.getConnection());
    subjectCombo->addItem("All Subjects", -1);
    for (auto &s : subjectDAO.getAllSubjects()) {
        subjectCombo->addItem(
            QString::fromStdString(s.subjectCode + " - " + s.subjectName), s.subjectId);
    }
}

void AttendanceRecordsWindow::refreshView()
{
    Database db(appDbPath());
    db.initializeTables();
    AttendanceDAO attendanceDAO(db.getConnection());
    SubjectDAO subjectDAO(db.getConnection());

    int subjectId = subjectCombo->currentData().toInt();

    QDate today = QDate::currentDate();
    QDate startDate = today;
    QString rangeName = rangeCombo->currentText();
    if (rangeName == "This Week") {
        startDate = today.addDays(-(today.dayOfWeek() - 1));
    } else if (rangeName == "This Month") {
        startDate = QDate(today.year(), today.month(), 1);
    } else if (rangeName == "All Time") {
        startDate = QDate(2000, 1, 1);
    }

    auto records = attendanceDAO.getDisplayRecords(
        startDate.toString("yyyy-MM-dd").toStdString(),
        today.toString("yyyy-MM-dd").toStdString(),
        subjectId);

    rosterTable->setRowCount(static_cast<int>(records.size()));
    for (int i = 0; i < static_cast<int>(records.size()); ++i) {
        auto &r = records[i];
        rosterTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(r.displayStudentName)));
        rosterTable->setItem(i, 1, new QTableWidgetItem(QString::number(r.displayRollNumber)));
        rosterTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(r.displaySessionDate)));
        rosterTable->setItem(i, 3, new QTableWidgetItem(
            QString::fromStdString(r.displayAttendanceTime.substr(0, 5))));
    }

    rangeSummaryLabel->setText(
        QString("Showing %1 record(s) from %2 to %3.")
            .arg(records.size())
            .arg(startDate.toString("MMM d, yyyy"))
            .arg(today.toString("MMM d, yyyy")));

    // Attendance % is only meaningful scoped to a single subject (it's computed
    // against that subject's own scheduled sessions), so only populate it then.
    percentTable->setRowCount(0);
    if (subjectId < 0) {
        return;
    }

    SubjectRecord subject = subjectDAO.getSubjectById(subjectId);
    int minAttendance = (subject.subjectId >= 0) ? subject.subjectMinAttendance : 80;

    auto percentages = attendanceDAO.getSubjectAttendancePercentage(subjectId);
    percentTable->setRowCount(static_cast<int>(percentages.size()));
    for (int i = 0; i < static_cast<int>(percentages.size()); ++i) {
        auto &p = percentages[i];
        percentTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(p.percentageStudentName)));
        percentTable->setItem(i, 1, new QTableWidgetItem(QString::number(p.percentageRollNumber)));

        QTableWidgetItem *pctItem = new QTableWidgetItem(
            QString::number(p.calculatedPercentage, 'f', 1) + "%");
        percentTable->setItem(i, 2, pctItem);

        bool belowThreshold = p.calculatedPercentage < minAttendance;
        QTableWidgetItem *statusItem = new QTableWidgetItem(
            belowThreshold ? QString("Below %1%% Minimum").arg(minAttendance) : "OK");
        statusItem->setForeground(belowThreshold ? QColor(Theme::Danger) : QColor(Theme::Success));
        statusItem->setFont(FontManager::buttonFont(13));
        percentTable->setItem(i, 3, statusItem);
    }
}
