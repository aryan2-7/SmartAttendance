#include "ScheduleEditor.h"
#include "../theme/Theme.h"
#include "../auth/FontManager.h"
#include "AdminDashboard.h"
#include "../db/SubjectDAO.h"
#include "../db/DbPath.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QLineEdit>
#include <QSpinBox>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QFrame>
#include <QAbstractItemView>
#include <QScrollArea>
#include <QDate>


ScheduleEditor::ScheduleEditor(QWidget *parent)
    : QWidget(parent), dbConn("") {
    dbConn = Database(std::string(PROJECT_SOURCE_DIR) + "/smart_attendance.db");
    dbConn.initializeTables();

    setupUI();
    setupStyles();

    SubjectDAO subjectDAO(dbConn.getConnection());
    std::vector<SubjectRecord> subjects = subjectDAO.getAllSubjects();
    subjectCombo->addItem("-- Select Subject --", -1);
    for (auto &s : subjects) {
        QString label = QString::fromStdString(s.subjectCode + " - " + s.subjectName);
        subjectCombo->addItem(label, s.subjectId);
    }
}

// =====================================
// Setup UI
// =====================================

void ScheduleEditor::setupUI() {
    setWindowTitle("Schedule Editor");
    resize(1200, 750);

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *scrollContent = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(scrollContent);
    mainLayout->setContentsMargins(40, 30, 40, 40);
    mainLayout->setSpacing(20);

    // =========================
    // Header
    // =========================

    QHBoxLayout *headerLayout = new QHBoxLayout();

    backButton = new QPushButton("← Back");
    backButton->setFixedSize(100, 42);

    QLabel *titleLabel = new QLabel("SCHEDULE EDITOR");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFont(FontManager::headingFont(22));

    headerLayout->addWidget(backButton);
    headerLayout->addStretch();
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    mainLayout->addLayout(headerLayout);

    // =========================
    // Subject Selection
    // =========================

    QHBoxLayout *subjectLayout = new QHBoxLayout();

    QLabel *subjectLabel = new QLabel("Subject:");

    subjectCombo = new QComboBox();
    subjectCombo->setMinimumWidth(350);

    connect(subjectCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ScheduleEditor::onSubjectSelected);

    subjectLayout->addWidget(subjectLabel);
    subjectLayout->addWidget(subjectCombo);
    subjectLayout->addStretch();

    mainLayout->addLayout(subjectLayout);

    // =========================
    // Add Session Card
    // =========================

    QGroupBox *sessionGroup = new QGroupBox("Add Session");
    sessionGroup->setObjectName("sessionGroup");

    QFormLayout *formLayout = new QFormLayout(sessionGroup);
    formLayout->setContentsMargins(25, 25, 25, 25);
    formLayout->setSpacing(15);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("yyyy-MM-dd");
    dateEdit->setMinimumWidth(220);
    formLayout->addRow("Date:", dateEdit);

    startTimeEdit = new QTimeEdit(QTime(8, 0));
    startTimeEdit->setDisplayFormat("HH:mm");
    startTimeEdit->setMinimumWidth(220);
    formLayout->addRow("Start Time:", startTimeEdit);

    endTimeEdit = new QTimeEdit(QTime(9, 0));
    endTimeEdit->setDisplayFormat("HH:mm");
    endTimeEdit->setMinimumWidth(220);
    formLayout->addRow("End Time:", endTimeEdit);

    roomEdit = new QLineEdit();
    roomEdit->setPlaceholderText("e.g. Lab 3, Room 201");
    roomEdit->setMinimumWidth(300);
    formLayout->addRow("Room:", roomEdit);

    topicEdit = new QLineEdit();
    topicEdit->setPlaceholderText("e.g. Chapter 5: Vectors");
    topicEdit->setMinimumWidth(300);
    formLayout->addRow("Topic:", topicEdit);

    mainLayout->addWidget(sessionGroup);

    // =========================
    // Weekly Generation Card
    // =========================

    QGroupBox *weeklyGroup = new QGroupBox("Add Weekly (Generate N Weeks)");
    weeklyGroup->setObjectName("sessionGroup");

    QHBoxLayout *weeklyLayout = new QHBoxLayout(weeklyGroup);
    weeklyLayout->setContentsMargins(25, 25, 25, 25);
    weeklyLayout->setSpacing(12);

    QLabel *weeksLabel = new QLabel("Weeks:");

    weeksSpin = new QSpinBox();
    weeksSpin->setRange(1, 16);
    weeksSpin->setValue(8);
    weeksSpin->setMinimumWidth(80);

    addWeeklyBtn = new QPushButton("Generate Weekly Sessions");
    addWeeklyBtn->setFixedHeight(42);

    weeklyLayout->addWidget(weeksLabel);
    weeklyLayout->addWidget(weeksSpin);
    weeklyLayout->addWidget(addWeeklyBtn);
    weeklyLayout->addStretch();

    mainLayout->addWidget(weeklyGroup);

    connect(addWeeklyBtn, &QPushButton::clicked, this, &ScheduleEditor::onAddWeeklyClicked);

    // =========================
    // Action Buttons
    // =========================

    QHBoxLayout *actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(12);

    addSessionBtn = new QPushButton("Add Single Session");
    deleteBtn = new QPushButton("Delete Selected Session");

    addSessionBtn->setFixedHeight(42);
    deleteBtn->setFixedHeight(42);

    actionLayout->addWidget(addSessionBtn);
    actionLayout->addWidget(deleteBtn);
    actionLayout->addStretch();

    mainLayout->addLayout(actionLayout);

    connect(addSessionBtn, &QPushButton::clicked, this, &ScheduleEditor::onAddSessionClicked);
    connect(deleteBtn, &QPushButton::clicked, this, &ScheduleEditor::onDeleteSessionClicked);

    // =========================
    // Status Label
    // =========================

    statusLabel = new QLabel();
    mainLayout->addWidget(statusLabel);

    // =========================
    // Session Table
    // =========================

    QLabel *sessionTitle = new QLabel("Scheduled Sessions");
    sessionTitle->setFont(FontManager::headingFont(18));
    mainLayout->addWidget(sessionTitle);

    sessionTable = new QTableWidget();
    sessionTable->setColumnCount(6);
    sessionTable->setHorizontalHeaderLabels(
        {"ID", "Date", "Start", "End", "Room", "Topic"});

    sessionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    sessionTable->setSelectionMode(QAbstractItemView::SingleSelection);
    sessionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    sessionTable->verticalHeader()->setVisible(false);
    sessionTable->horizontalHeader()->setStretchLastSection(true);
    sessionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    sessionTable->setMinimumHeight(300);

    mainLayout->addWidget(sessionTable, 1);

    scrollArea->setWidget(scrollContent);
    outerLayout->addWidget(scrollArea);

    // =========================
    // Connections
    // =========================

    connect(backButton, &QPushButton::clicked, this, [this]() {
        auto *window = new AdminDashboard();
        window->showMaximized();
        this->close();
    });
}

// =====================================
// Styling
// =====================================

void ScheduleEditor::setupStyles() {
    setStyleSheet(QString(R"(
        QWidget {
            background:%1;
            color:%2;
        }

        QLabel {
            color:%2;
        }

        QGroupBox#sessionGroup {
            background:%3;
            border:1px solid %4;
            border-radius:18px;
            margin-top:12px;
            font-weight:bold;
            color:%2;
        }

        QGroupBox#sessionGroup::title {
            subcontrol-origin:margin;
            left:16px;
            padding:0 6px;
            color:%6;
        }

        QLineEdit,
        QComboBox,
        QDateEdit,
        QTimeEdit,
        QSpinBox {
            background:%5;
            border:1px solid %4;
            border-radius:10px;
            padding:10px 16px;
            color:%2;
            min-height:20px;
        }

        QLineEdit:focus,
        QComboBox:focus,
        QDateEdit:focus,
        QTimeEdit:focus,
        QSpinBox:focus {
            border:1px solid %6;
        }

        QComboBox QAbstractItemView {
            background:%5;
            border:1px solid %4;
            border-radius:6px;
            color:%2;
            min-width:400px;
            selection-background-color:%3;
            selection-color:%2;
            padding:4px;
        }

        QComboBox QAbstractItemView::item {
            padding:6px 12px;
        }

        QScrollBar:vertical {
            background:%5;
            width:10px;
            border-radius:5px;
        }

        QScrollBar::handle:vertical {
            background:%4;
            border-radius:5px;
            min-height:30px;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height:0px;
        }

        QPushButton {
            background:%3;
            color:%2;
            border:1px solid %4;
            border-radius:10px;
            padding:8px 18px;
            font-weight:bold;
        }

        QPushButton:hover {
            background:%7;
        }

        QLabel#statusLabel,
        QLabel {
            color:%2;
        }

        QTableWidget {
            background:%5;
            border:1px solid %4;
            border-radius:14px;
            gridline-color:%4;
            color:%2;
            selection-background-color:%3;
            selection-color:%2;
        }

        QTableWidget::item {
            padding:10px;
        }

        QHeaderView::section {
            background:%3;
            color:%2;
            padding:12px;
            border:none;
            border-bottom:1px solid %4;
            font-weight:bold;
        }
    )")
                      .arg(Theme::Card)
                      .arg(Theme::Primary)
                      .arg(Theme::Surface)
                      .arg(Theme::Border)
                      .arg(Theme::Input)
                      .arg(Theme::Gold)
                      .arg(Theme::Hover));

    statusLabel->setStyleSheet(QString("QLabel{ color:%1; padding:4px; font-weight:bold; }")
                                    .arg(Theme::Success));
}

// =====================================
// Logic (unchanged from integration)
// =====================================

void ScheduleEditor::onSubjectSelected(int /*index*/) {
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