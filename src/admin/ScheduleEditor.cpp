#include "ScheduleEditor.h"
#include "../theme/Theme.h"
#include "../auth/FontManager.h"
#include "AdminDashboard.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>

#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QFrame>
#include <QAbstractItemView>
#include <QInputDialog>


ScheduleEditor::ScheduleEditor(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupStyles();
    loadSessions();
}


// =====================================
// Setup UI
// =====================================

void ScheduleEditor::setupUI()
{
    setWindowTitle("Schedule Editor");
    resize(1200, 750);

    // =========================
    // Main Layout
    // =========================

    QVBoxLayout *mainLayout =
        new QVBoxLayout(this);

    mainLayout->setContentsMargins(
        40, 30, 40, 30
        );

    mainLayout->setSpacing(20);


    // =========================
    // Header
    // =========================

    QHBoxLayout *headerLayout =
        new QHBoxLayout();

    backButton =
        new QPushButton("← Back");

    backButton->setFixedSize(
        100, 42
        );

    QLabel *titleLabel =
        new QLabel("SCHEDULE EDITOR");

    titleLabel->setAlignment(
        Qt::AlignCenter
        );

    titleLabel->setFont(
        FontManager::headingFont(22)
        );

    headerLayout->addWidget(
        backButton
        );

    headerLayout->addStretch();

    headerLayout->addWidget(
        titleLabel
        );

    headerLayout->addStretch();


    // =========================
    // Subject Selection
    // =========================

    QHBoxLayout *subjectLayout =
        new QHBoxLayout();

    QLabel *subjectLabel =
        new QLabel("Subject:");

    subjectCombo =
        new QComboBox();

    subjectCombo->addItem(
        "Select Subject"
        );

    // Temporary subject data
    subjectCombo->addItem(
        "CS101 - Programming Fundamentals"
        );

    subjectCombo->addItem(
        "CS201 - Data Structures"
        );

    subjectCombo->addItem(
        "CS301 - Database Systems"
        );

    subjectCombo->addItem(
        "CS401 - Computer Networks"
        );

    subjectLayout->addWidget(
        subjectLabel
        );

    subjectLayout->addWidget(
        subjectCombo
        );

    subjectLayout->addStretch();


    // =========================
    // Schedule Form Card
    // =========================

    QFrame *formCard =
        new QFrame();

    formCard->setObjectName(
        "formCard"
        );

    QFormLayout *formLayout =
        new QFormLayout(formCard);

    formLayout->setContentsMargins(
        25, 25, 25, 25
        );

    formLayout->setSpacing(15);


    // Date

    dateEdit =
        new QDateEdit();

    dateEdit->setCalendarPopup(
        true
        );

    dateEdit->setDate(
        QDate::currentDate()
        );


    // Start Time

    startTimeEdit =
        new QTimeEdit();

    startTimeEdit->setTime(
        QTime(10, 0)
        );


    // End Time

    endTimeEdit =
        new QTimeEdit();

    endTimeEdit->setTime(
        QTime(11, 0)
        );


    // Room

    roomEdit =
        new QLineEdit();

    roomEdit->setPlaceholderText(
        "Enter room number"
        );


    // Topic

    topicEdit =
        new QLineEdit();

    topicEdit->setPlaceholderText(
        "Enter session topic"
        );


    // Add fields

    formLayout->addRow(
        "Date:",
        dateEdit
        );

    formLayout->addRow(
        "Start Time:",
        startTimeEdit
        );

    formLayout->addRow(
        "End Time:",
        endTimeEdit
        );

    formLayout->addRow(
        "Room:",
        roomEdit
        );

    formLayout->addRow(
        "Topic:",
        topicEdit
        );


    // =========================
    // Action Buttons
    // =========================

    QHBoxLayout *actionLayout =
        new QHBoxLayout();

    addSessionButton =
        new QPushButton("Add Session");

    addWeeklyButton =
        new QPushButton("Add Weekly");


    addSessionButton->setFixedHeight(
        42
        );

    addWeeklyButton->setFixedHeight(
        42
        );


    actionLayout->addStretch();

    actionLayout->addWidget(
        addSessionButton
        );

    actionLayout->addWidget(
        addWeeklyButton
        );


    formLayout->addRow(
        actionLayout
        );


    // =========================
    // Session Table
    // =========================

    QLabel *sessionTitle =
        new QLabel("Scheduled Sessions");

    sessionTitle->setFont(
        FontManager::headingFont(18)
        );


    sessionTable =
        new QTableWidget();

    sessionTable->setColumnCount(5);

    sessionTable->setHorizontalHeaderLabels({
        "Date",
        "Time",
        "Room",
        "Topic",
        "Subject"
    });


    sessionTable->setSelectionBehavior(
        QAbstractItemView::SelectRows
        );

    sessionTable->setSelectionMode(
        QAbstractItemView::SingleSelection
        );

    sessionTable->setEditTriggers(
        QAbstractItemView::NoEditTriggers
        );

    sessionTable->verticalHeader()->setVisible(
        false
        );

    sessionTable->horizontalHeader()
        ->setSectionResizeMode(
            QHeaderView::Stretch
            );


    // =========================
    // Delete Button
    // =========================

    QHBoxLayout *deleteLayout =
        new QHBoxLayout();

    deleteButton =
        new QPushButton("Delete Session");

    deleteButton->setFixedSize(
        150, 42
        );

    deleteLayout->addStretch();

    deleteLayout->addWidget(
        deleteButton
        );


    // =========================
    // Add Everything
    // =========================

    mainLayout->addLayout(
        headerLayout
        );

    mainLayout->addLayout(
        subjectLayout
        );

    mainLayout->addWidget(
        formCard
        );

    mainLayout->addWidget(
        sessionTitle
        );

    mainLayout->addWidget(
        sessionTable
        );

    mainLayout->addLayout(
        deleteLayout
        );


    // =========================
    // Connections
    // =========================

    connect(backButton, &QPushButton::clicked, this, [this]()
            {
                auto *window = new AdminDashboard();
                window->show();
                this->close();
            });
    connect(
        addSessionButton,
        &QPushButton::clicked,
        this,
        &ScheduleEditor::onAddSession
        );

    connect(
        addWeeklyButton,
        &QPushButton::clicked,
        this,
        &ScheduleEditor::onAddWeekly
        );

    connect(
        deleteButton,
        &QPushButton::clicked,
        this,
        &ScheduleEditor::onDeleteSession
        );
}


// =====================================
// Styling
// =====================================

void ScheduleEditor::setupStyles()
{
    setStyleSheet(QString(R"(
        QWidget {
            background:%1;
            color:%2;
            font-family:"Segoe UI";
        }

        QLabel {
            color:%2;
        }

        QFrame#formCard {
            background:%3;
            border:1px solid %4;
            border-radius:18px;
        }

        QLineEdit,
        QComboBox,
        QDateEdit,
        QTimeEdit {
            background:%5;
            border:1px solid %4;
            border-radius:10px;
            padding:8px 12px;
            color:%2;
            min-height:20px;
        }

        QLineEdit:focus,
        QComboBox:focus,
        QDateEdit:focus,
        QTimeEdit:focus {
            border:1px solid %6;
        }

        QPushButton {
            background:%3;
            color:%2;
            border:1px solid %4;
            border-radius:10px;
            padding:8px 18px;
        }

        QPushButton:hover {
            background:%7;
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
}


// =====================================
// Temporary Session Data
// =====================================

void ScheduleEditor::loadSessions()
{
    // Temporary data for UI testing.

    sessionTable->setRowCount(2);

    sessionTable->setItem(
        0, 0,
        new QTableWidgetItem("2026-07-25")
        );

    sessionTable->setItem(
        0, 1,
        new QTableWidgetItem("10:00 - 11:00")
        );

    sessionTable->setItem(
        0, 2,
        new QTableWidgetItem("Room 301")
        );

    sessionTable->setItem(
        0, 3,
        new QTableWidgetItem("Introduction to Programming")
        );

    sessionTable->setItem(
        0, 4,
        new QTableWidgetItem("CS101")
        );


    sessionTable->setItem(
        1, 0,
        new QTableWidgetItem("2026-07-27")
        );

    sessionTable->setItem(
        1, 1,
        new QTableWidgetItem("10:00 - 11:00")
        );

    sessionTable->setItem(
        1, 2,
        new QTableWidgetItem("Room 301")
        );

    sessionTable->setItem(
        1, 3,
        new QTableWidgetItem("Variables and Data Types")
        );

    sessionTable->setItem(
        1, 4,
        new QTableWidgetItem("CS101")
        );
}


void ScheduleEditor::onAddSession()
{
    // =========================
    // Validate Subject
    // =========================

    if (subjectCombo->currentIndex() == 0) {
        QMessageBox::warning(
            this,
            "No Subject Selected",
            "Please select a subject first."
            );
        return;
    }


    // =========================
    // Validate Room
    // =========================

    QString room = roomEdit->text().trimmed();

    if (room.isEmpty()) {
        QMessageBox::warning(
            this,
            "Missing Room",
            "Please enter the room number."
            );
        return;
    }


    // =========================
    // Validate Topic
    // =========================

    QString topic = topicEdit->text().trimmed();

    if (topic.isEmpty()) {
        QMessageBox::warning(
            this,
            "Missing Topic",
            "Please enter the session topic."
            );
        return;
    }


    // =========================
    // Validate Time
    // =========================

    if (startTimeEdit->time() >= endTimeEdit->time()) {
        QMessageBox::warning(
            this,
            "Invalid Time",
            "End time must be later than start time."
            );
        return;
    }


    // =========================
    // Get Form Data
    // =========================

    QString date =
        dateEdit->date().toString("yyyy-MM-dd");

    QString startTime =
        startTimeEdit->time().toString("HH:mm");

    QString endTime =
        endTimeEdit->time().toString("HH:mm");

    QString time =
        startTime + " - " + endTime;

    QString subject =
        subjectCombo->currentText();


    // =========================
    // Add New Row
    // =========================

    int row =
        sessionTable->rowCount();

    sessionTable->insertRow(row);


    sessionTable->setItem(
        row,
        0,
        new QTableWidgetItem(date)
        );

    sessionTable->setItem(
        row,
        1,
        new QTableWidgetItem(time)
        );

    sessionTable->setItem(
        row,
        2,
        new QTableWidgetItem(room)
        );

    sessionTable->setItem(
        row,
        3,
        new QTableWidgetItem(topic)
        );

    sessionTable->setItem(
        row,
        4,
        new QTableWidgetItem(subject)
        );


    // =========================
    // Clear Form
    // =========================

    roomEdit->clear();
    topicEdit->clear();


    // =========================
    // Confirmation
    // =========================

    QMessageBox::information(
        this,
        "Session Added",
        "The session has been added successfully."
        );
}


void ScheduleEditor::onAddWeekly()
{
    // Check subject selection
    if (subjectCombo->currentIndex() == 0) {
        QMessageBox::warning(
            this,
            "No Subject Selected",
            "Please select a subject first."
            );
        return;
    }

    // Check room
    if (roomEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(
            this,
            "Missing Room",
            "Please enter the room number."
            );
        return;
    }

    // Check topic
    if (topicEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(
            this,
            "Missing Topic",
            "Please enter the session topic."
            );
        return;
    }

    // Check time
    if (startTimeEdit->time() >= endTimeEdit->time()) {
        QMessageBox::warning(
            this,
            "Invalid Time",
            "End time must be later than start time."
            );
        return;
    }

    // Ask how many weeks
    bool ok = false;

    int weeks = QInputDialog::getInt(
        this,
        "Add Weekly Sessions",
        "Number of weeks:",
        4,      // Default
        1,      // Minimum
        52,     // Maximum
        1,      // Step
        &ok
        );

    if (!ok) {
        return;
    }

    // Get selected values
    QDate startDate = dateEdit->date();

    QString startTime =
        startTimeEdit->time().toString("HH:mm");

    QString endTime =
        endTimeEdit->time().toString("HH:mm");

    QString room =
        roomEdit->text().trimmed();

    QString topic =
        topicEdit->text().trimmed();

    QString subject =
        subjectCombo->currentText();

    // Add one session per week
    for (int i = 0; i < weeks; ++i) {

        QDate sessionDate =
            startDate.addDays(i * 7);

        int row =
            sessionTable->rowCount();

        sessionTable->insertRow(row);

        sessionTable->setItem(
            row,
            0,
            new QTableWidgetItem(
                sessionDate.toString("yyyy-MM-dd")
                )
            );

        sessionTable->setItem(
            row,
            1,
            new QTableWidgetItem(
                startTime + " - " + endTime
                )
            );

        sessionTable->setItem(
            row,
            2,
            new QTableWidgetItem(room)
            );

        sessionTable->setItem(
            row,
            3,
            new QTableWidgetItem(topic)
            );

        sessionTable->setItem(
            row,
            4,
            new QTableWidgetItem(subject)
            );
    }

    QMessageBox::information(
        this,
        "Weekly Schedule Added",
        QString("%1 weekly sessions have been added successfully.")
            .arg(weeks)
        );
}


void ScheduleEditor::onDeleteSession()
{
    // =========================
    // Check Selection
    // =========================

    int selectedRow =
        sessionTable->currentRow();

    if (selectedRow < 0) {
        QMessageBox::warning(
            this,
            "No Session Selected",
            "Please select a session to delete."
            );
        return;
    }


    // =========================
    // Get Session Information
    // =========================

    QString date =
        sessionTable->item(selectedRow, 0)
            ->text();

    QString time =
        sessionTable->item(selectedRow, 1)
            ->text();

    QString room =
        sessionTable->item(selectedRow, 2)
            ->text();

    QString topic =
        sessionTable->item(selectedRow, 3)
            ->text();

    QString subject =
        sessionTable->item(selectedRow, 4)
            ->text();


    // =========================
    // Confirmation Dialog
    // =========================

    QMessageBox::StandardButton reply =
        QMessageBox::question(
            this,
            "Delete Session",
            "Are you sure you want to delete this session?\n\n"
            "Subject: " + subject +
                "\nDate: " + date +
                "\nTime: " + time +
                "\nRoom: " + room +
                "\nTopic: " + topic,

            QMessageBox::Yes |
                QMessageBox::No,

            QMessageBox::No
            );


    // =========================
    // Delete Row
    // =========================

    if (reply == QMessageBox::Yes) {

        sessionTable->removeRow(
            selectedRow
            );

        QMessageBox::information(
            this,
            "Session Deleted",
            "The session has been deleted successfully."
            );
    }
}