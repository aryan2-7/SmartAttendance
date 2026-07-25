#include "ScheduleEditor.h"
#include "../db/SubjectDAO.h"
#include "../theme/Theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>

static const int SECONDS_IN_DAY = 86400;

ScheduleEditor::ScheduleEditor(QWidget *parent)
    : QWidget(parent), dbConn("") {
    dbConn = Database(std::string(PROJECT_SOURCE_DIR) + "/smart_attendance.db");
    dbConn.initializeTables();
    setupUI();

    SubjectDAO subjectDAO(dbConn.getConnection());
    std::vector<SubjectRecord> subjects = subjectDAO.getAllSubjects();
    subjectCombo->addItem("-- Select Subject --", -1);
    for (auto &s : subjects) {
        QString label = QString::fromStdString(s.subjectCode + " - " + s.subjectName);
        subjectCombo->addItem(label, s.subjectId);
    }
}

void ScheduleEditor::setupUI() {
    setStyleSheet(QString("QWidget{ background:%1; color:%2; }")
                      .arg(Theme::Card).arg(Theme::Primary));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    QLabel *title = new QLabel("Schedule Editor");
    QFont titleFont;
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setStyleSheet(QString("color:%1; padding-bottom:8px;").arg(Theme::Gold));
    mainLayout->addWidget(title);

    subjectCombo = new QComboBox();
    subjectCombo->setStyleSheet(QString(
        "QComboBox{ background:%1; color:%2; border:1px solid %3; border-radius:6px; padding:6px; }")
        .arg(Theme::Input).arg(Theme::Primary).arg(Theme::Border));
    mainLayout->addWidget(subjectCombo);
    connect(subjectCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ScheduleEditor::onSubjectSelected);

    QGroupBox *sessionGroup = new QGroupBox("Add Session");
    sessionGroup->setStyleSheet(QString(
        "QGroupBox{ background:%1; border:1px solid %2; border-radius:8px; "
        "margin-top:16px; padding-top:20px; font-weight:bold; color:%3; }"
        "QGroupBox::title{ subcontrol-origin:margin; left:12px; padding:0 4px; }")
        .arg(Theme::Surface).arg(Theme::Border).arg(Theme::Primary));

    QFormLayout *formLayout = new QFormLayout(sessionGroup);
    formLayout->setSpacing(10);
    formLayout->setContentsMargins(16, 24, 16, 16);

    QString fieldStyle = QString(
        "QDateEdit,QTimeEdit,QLineEdit,QSpinBox{ background:%1; color:%2; "
        "border:1px solid %3; border-radius:6px; padding:6px; }")
        .arg(Theme::Input).arg(Theme::Primary).arg(Theme::Border);

    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("yyyy-MM-dd");
    dateEdit->setStyleSheet(fieldStyle);
    formLayout->addRow("Date:", dateEdit);

    startTimeEdit = new QTimeEdit(QTime(8, 0));
    startTimeEdit->setDisplayFormat("HH:mm");
    startTimeEdit->setStyleSheet(fieldStyle);
    formLayout->addRow("Start Time:", startTimeEdit);

    endTimeEdit = new QTimeEdit(QTime(9, 0));
    endTimeEdit->setDisplayFormat("HH:mm");
    endTimeEdit->setStyleSheet(fieldStyle);
    formLayout->addRow("End Time:", endTimeEdit);

    roomEdit = new QLineEdit();
    roomEdit->setPlaceholderText("e.g. Lab 3, Room 201");
    roomEdit->setStyleSheet(fieldStyle);
    formLayout->addRow("Room:", roomEdit);

    topicEdit = new QLineEdit();
    topicEdit->setPlaceholderText("e.g. Chapter 5: Vectors");
    topicEdit->setStyleSheet(fieldStyle);
    formLayout->addRow("Topic:", topicEdit);

    mainLayout->addWidget(sessionGroup);

    QGroupBox *weeklyGroup = new QGroupBox("Add Weekly (Generate N Weeks)");
    weeklyGroup->setStyleSheet(sessionGroup->styleSheet());

    QHBoxLayout *weeklyLayout = new QHBoxLayout(weeklyGroup);
    weeklyLayout->setContentsMargins(16, 24, 16, 16);
    weeklyLayout->setSpacing(12);

    QLabel *weeksLabel = new QLabel("Weeks:");
    weeksLabel->setStyleSheet(QString("color:%1;").arg(Theme::Primary));

    weeksSpin = new QSpinBox();
    weeksSpin->setRange(1, 16);
    weeksSpin->setValue(8);
    weeksSpin->setStyleSheet(fieldStyle);

    addWeeklyBtn = new QPushButton("Generate Weekly Sessions");
    addWeeklyBtn->setStyleSheet(QString(
        "QPushButton{ background:%1; color:%2; border:1px solid %3; "
        "border-radius:8px; padding:10px 20px; font-weight:bold; }"
        "QPushButton:hover{ background:%4; }")
        .arg(Theme::Surface).arg(Theme::Primary).arg(Theme::Border).arg(Theme::Hover));

    weeklyLayout->addWidget(weeksLabel);
    weeklyLayout->addWidget(weeksSpin);
    weeklyLayout->addWidget(addWeeklyBtn);
    weeklyLayout->addStretch();
    mainLayout->addWidget(weeklyGroup);

    connect(addWeeklyBtn, &QPushButton::clicked, this, &ScheduleEditor::onAddWeeklyClicked);

    QHBoxLayout *actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(12);

    addSessionBtn = new QPushButton("Add Single Session");
    deleteBtn = new QPushButton("Delete Selected Session");

    QString btnStyle = QString(
        "QPushButton{ background:%1; color:%2; border:1px solid %3; "
        "border-radius:8px; padding:10px 20px; font-weight:bold; }"
        "QPushButton:hover{ background:%4; }")
        .arg(Theme::Surface).arg(Theme::Primary).arg(Theme::Border).arg(Theme::Hover);

    addSessionBtn->setStyleSheet(btnStyle);
    deleteBtn->setStyleSheet(btnStyle);

    actionLayout->addWidget(addSessionBtn);
    actionLayout->addWidget(deleteBtn);
    actionLayout->addStretch();
    mainLayout->addLayout(actionLayout);

    connect(addSessionBtn, &QPushButton::clicked, this, &ScheduleEditor::onAddSessionClicked);
    connect(deleteBtn, &QPushButton::clicked, this, &ScheduleEditor::onDeleteSessionClicked);

    statusLabel = new QLabel();
    statusLabel->setStyleSheet(QString("QLabel{ color:%1; padding:4px; }").arg(Theme::Success));
    mainLayout->addWidget(statusLabel);

    sessionTable = new QTableWidget();
    sessionTable->setColumnCount(6);
    sessionTable->setHorizontalHeaderLabels(
        {"ID", "Date", "Start", "End", "Room", "Topic"});
    sessionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    sessionTable->setSelectionMode(QAbstractItemView::SingleSelection);
    sessionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    sessionTable->horizontalHeader()->setStretchLastSection(true);
    sessionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    sessionTable->verticalHeader()->hide();
    sessionTable->setStyleSheet(QString(
        "QTableWidget{ background:%1; color:%2; border:1px solid %3; "
        "border-radius:6px; gridline-color:%4; }"
        "QTableWidget::item{ padding:6px; }"
        "QHeaderView::section{ background:%5; color:%2; border:1px solid %3; padding:6px; font-weight:bold; }")
        .arg(Theme::Input).arg(Theme::Primary).arg(Theme::Border)
        .arg(Theme::Border).arg(Theme::Surface));

    mainLayout->addWidget(sessionTable);
}

void ScheduleEditor::onSubjectSelected(int index) {
    refreshSessionList();
}

void ScheduleEditor::refreshSessionList() {
    int subjectId = subjectCombo->currentData().toInt();
    if (subjectId < 0) {
        sessionTable->setRowCount(0);
        return;
    }

    SubjectDAO subjectDAO(dbConn.getConnection());
    std::vector<ClassSessionRecord> sessions = subjectDAO.getSessionsForSubject(subjectId);

    sessionTable->setRowCount(static_cast<int>(sessions.size()));
    for (int i = 0; i < static_cast<int>(sessions.size()); ++i) {
        auto &s = sessions[i];
        sessionTable->setItem(i, 0, new QTableWidgetItem(QString::number(s.sessionId)));
        sessionTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(s.sessionDate)));
        sessionTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(s.sessionStartTime)));
        sessionTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(s.sessionEndTime)));
        sessionTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(s.sessionRoom)));
        sessionTable->setItem(i, 5, new QTableWidgetItem(QString::fromStdString(s.sessionTopic)));
    }
}

void ScheduleEditor::addSingleSession(int subjectId, const std::string &date,
                                      const std::string &start, const std::string &end,
                                      const std::string &room, const std::string &topic) {
    SubjectDAO subjectDAO(dbConn.getConnection());
    subjectDAO.createClassSession(subjectId, date, start, end, room, topic);
}

void ScheduleEditor::onAddSessionClicked() {
    int subjectId = subjectCombo->currentData().toInt();
    if (subjectId < 0) {
        statusLabel->setText("Error: Please select a subject first.");
        return;
    }

    std::string date = dateEdit->date().toString("yyyy-MM-dd").toStdString();
    std::string start = startTimeEdit->time().toString("HH:mm").toStdString();
    std::string end = endTimeEdit->time().toString("HH:mm").toStdString();
    std::string room = roomEdit->text().trimmed().toStdString();
    std::string topic = topicEdit->text().trimmed().toStdString();

    if (start >= end) {
        statusLabel->setText("Error: Start time must be before end time.");
        return;
    }

    addSingleSession(subjectId, date, start, end, room, topic);
    statusLabel->setText("Session added successfully.");
    refreshSessionList();
    emit scheduleChanged();
}

void ScheduleEditor::onAddWeeklyClicked() {
    int subjectId = subjectCombo->currentData().toInt();
    if (subjectId < 0) {
        statusLabel->setText("Error: Please select a subject first.");
        return;
    }

    int numWeeks = weeksSpin->value();
    QDate startDate = dateEdit->date();
    int dayOfWeek = startDate.dayOfWeek();

    std::string start = startTimeEdit->time().toString("HH:mm").toStdString();
    std::string end = endTimeEdit->time().toString("HH:mm").toStdString();
    std::string room = roomEdit->text().trimmed().toStdString();
    std::string topic = topicEdit->text().trimmed().toStdString();

    if (start >= end) {
        statusLabel->setText("Error: Start time must be before end time.");
        return;
    }

    int created = 0;
    for (int w = 0; w < numWeeks; ++w) {
        QDate sessionDate = startDate.addDays(w * 7);
        if (sessionDate.dayOfWeek() != dayOfWeek) {
            int diff = dayOfWeek - sessionDate.dayOfWeek();
            sessionDate = sessionDate.addDays(diff);
        }
        if (sessionDate < QDate::currentDate()) continue;

        addSingleSession(subjectId,
                         sessionDate.toString("yyyy-MM-dd").toStdString(),
                         start, end, room, topic);
        created++;
    }

    statusLabel->setText(QString("Generated %1 weekly session(s).").arg(created));
    refreshSessionList();
    emit scheduleChanged();
}

void ScheduleEditor::onDeleteSessionClicked() {
    int row = sessionTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Please select a session to delete.");
        return;
    }

    int sessionId = sessionTable->item(row, 0)->text().toInt();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete",
        "Delete this session and all its attendance records?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    SubjectDAO subjectDAO(dbConn.getConnection());
    if (subjectDAO.deleteClassSession(sessionId)) {
        statusLabel->setText("Session deleted.");
        refreshSessionList();
        emit scheduleChanged();
    } else {
        statusLabel->setText("Error: Failed to delete session.");
    }
}
