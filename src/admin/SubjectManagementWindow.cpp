#include "SubjectManagementWindow.h"
#include "../theme/Theme.h"
#include "../auth/FontManager.h"
#include "AdminDashboard.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QDialogButtonBox>

SubjectManagementWindow::SubjectManagementWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupStyles();
    loadSubjects();
}

void SubjectManagementWindow::setupUI()
{
    setWindowTitle("Subject Management");
    resize(1200, 750);

    // =========================
    // Main Layout
    // =========================

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);

    // =========================
    // Header
    // =========================

    QHBoxLayout *headerLayout = new QHBoxLayout();

    backButton = new QPushButton("← Back");
    backButton->setFixedSize(100, 42);

    QLabel *titleLabel = new QLabel("SUBJECT MANAGEMENT");

    titleLabel->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(backButton);
    headerLayout->addStretch();
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    // =========================
    // Search + Add Button
    // =========================

    QHBoxLayout *actionLayout = new QHBoxLayout();

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Search subjects...");
    searchEdit->setFixedHeight(45);

    addButton = new QPushButton("+ Add Subject");
    addButton->setFixedSize(150, 45);

    actionLayout->addWidget(searchEdit);
    actionLayout->addWidget(addButton);

    // =========================
    // Subject Table
    // =========================

    subjectTable = new QTableWidget();

    subjectTable->setColumnCount(5);

    subjectTable->setHorizontalHeaderLabels({
        "Subject Code",
        "Subject Name",
        "Semester",
        "Department",
        "Min Attendance (%)"
    });

    subjectTable->setSelectionBehavior(
        QAbstractItemView::SelectRows
        );

    subjectTable->setSelectionMode(
        QAbstractItemView::SingleSelection
        );

    subjectTable->setEditTriggers(
        QAbstractItemView::NoEditTriggers
        );

    subjectTable->setAlternatingRowColors(false);

    subjectTable->verticalHeader()->setVisible(false);

    subjectTable->horizontalHeader()->setStretchLastSection(true);

    subjectTable->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
        );

    // =========================
    // Bottom Buttons
    // =========================

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    editButton = new QPushButton("Edit Subject");
    deleteButton = new QPushButton("Delete Subject");

    editButton->setFixedSize(150, 45);
    deleteButton->setFixedSize(150, 45);

    buttonLayout->addStretch();
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);

    // =========================
    // Add to Main Layout
    // =========================

    mainLayout->addLayout(headerLayout);
    mainLayout->addLayout(actionLayout);
    mainLayout->addWidget(subjectTable);
    mainLayout->addLayout(buttonLayout);

    // =========================
    // Connections
    // =========================

    connect(addButton, &QPushButton::clicked,
            this, &SubjectManagementWindow::onAddSubject);

    connect(editButton, &QPushButton::clicked,
            this, &SubjectManagementWindow::onEditSubject);

    connect(deleteButton, &QPushButton::clicked,
            this, &SubjectManagementWindow::onDeleteSubject);

    connect(searchEdit, &QLineEdit::textChanged,
            this, &SubjectManagementWindow::onSearchSubject);

    connect(backButton, &QPushButton::clicked, this, [this]() {
        auto *dashboard = new AdminDashboard();
        dashboard->show();
        this->close();
    });
}


// =====================================
// Styling
// =====================================

void SubjectManagementWindow::setupStyles()
{
    setStyleSheet(R"(
        QWidget {
            background-color: #0F172A;
            color: #FFFFFF;
            font-family: "Segoe UI";
        }

        QLabel {
            color: #FFFFFF;
        }

        QLineEdit {
            background-color: #111827;
            border: 1px solid #334155;
            border-radius: 12px;
            padding: 0 15px;
            color: #FFFFFF;
            font-size: 14px;
        }

        QLineEdit:focus {
            border: 1px solid #3B82F6;
        }

        QPushButton {
            background-color: #1E293B;
            border: 1px solid #334155;
            border-radius: 10px;
            padding: 8px 16px;
            color: #FFFFFF;
            font-size: 14px;
        }

        QPushButton:hover {
            background-color: #334155;
        }

        QPushButton:pressed {
            background-color: #475569;
        }

        QTableWidget {
            background-color: #111827;
            border: 1px solid #334155;
            border-radius: 14px;
            gridline-color: #334155;
            color: #FFFFFF;
            selection-background-color: #1E3A5F;
            selection-color: #FFFFFF;
        }

        QTableWidget::item {
            padding: 10px;
        }

        QHeaderView::section {
            background-color: #1E293B;
            color: #FFFFFF;
            padding: 12px;
            border: none;
            border-bottom: 1px solid #334155;
            font-weight: bold;
        }

        QScrollBar:vertical {
            background-color: #111827;
            width: 10px;
            border-radius: 5px;
        }

        QScrollBar::handle:vertical {
            background-color: #334155;
            border-radius: 5px;
        }
    )");
}


// =====================================
// Temporary Subject Data
// =====================================

void SubjectManagementWindow::loadSubjects()
{
    // Temporary data for UI testing.
    // This will later be replaced by Shashwot's SubjectDAO logic.

    subjectTable->setRowCount(4);

    subjectTable->setItem(0, 0, new QTableWidgetItem("CS101"));
    subjectTable->setItem(0, 1, new QTableWidgetItem("Programming Fundamentals"));
    subjectTable->setItem(0, 2, new QTableWidgetItem("1"));
    subjectTable->setItem(0, 3, new QTableWidgetItem("Computer Science"));
    subjectTable->setItem(0, 4, new QTableWidgetItem("75"));

    subjectTable->setItem(1, 0, new QTableWidgetItem("CS201"));
    subjectTable->setItem(1, 1, new QTableWidgetItem("Data Structures"));
    subjectTable->setItem(1, 2, new QTableWidgetItem("2"));
    subjectTable->setItem(1, 3, new QTableWidgetItem("Computer Science"));
    subjectTable->setItem(1, 4, new QTableWidgetItem("75"));

    subjectTable->setItem(2, 0, new QTableWidgetItem("CS301"));
    subjectTable->setItem(2, 1, new QTableWidgetItem("Database Systems"));
    subjectTable->setItem(2, 2, new QTableWidgetItem("3"));
    subjectTable->setItem(2, 3, new QTableWidgetItem("Computer Science"));
    subjectTable->setItem(2, 4, new QTableWidgetItem("80"));

    subjectTable->setItem(3, 0, new QTableWidgetItem("CS401"));
    subjectTable->setItem(3, 1, new QTableWidgetItem("Computer Networks"));
    subjectTable->setItem(3, 2, new QTableWidgetItem("4"));
    subjectTable->setItem(3, 3, new QTableWidgetItem("Computer Science"));
    subjectTable->setItem(3, 4, new QTableWidgetItem("75"));
}


// =====================================
// Add Subject
// =====================================

void SubjectManagementWindow::onAddSubject()
{
    QDialog dialog(this);

    dialog.setWindowTitle("Add Subject");
    dialog.setModal(true);
    dialog.resize(450, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);

    // =========================
    // Title
    // =========================

    QLabel *titleLabel = new QLabel("ADD SUBJECT");

    titleLabel->setFont(FontManager::headingFont(20));
    titleLabel->setAlignment(Qt::AlignCenter);

    titleLabel->setStyleSheet(
        QString("color:%1;").arg(Theme::Gold)
        );

    mainLayout->addWidget(titleLabel);

    mainLayout->addSpacing(15);


    // =========================
    // Form
    // =========================

    QFormLayout *formLayout = new QFormLayout();

    formLayout->setSpacing(15);


    // Subject Code

    QLineEdit *codeEdit = new QLineEdit();

    codeEdit->setPlaceholderText(
        "Enter subject code"
        );


    // Subject Name

    QLineEdit *nameEdit = new QLineEdit();

    nameEdit->setPlaceholderText(
        "Enter subject name"
        );


    // Semester

    QComboBox *semesterCombo = new QComboBox();

    semesterCombo->addItem("Select Semester");

    for (int i = 1; i <= 8; ++i) {
        semesterCombo->addItem(
            QString("Semester %1").arg(i),
            i
            );
    }


    // Department

    QComboBox *departmentCombo = new QComboBox();

    departmentCombo->addItem(
        "Select Department"
        );

    departmentCombo->addItems({
        "Computer Engineering",
        "Civil Engineering",
        "Electrical Engineering",
        "Electronics Engineering",
        "Mechanical Engineering"
    });


    // Minimum Attendance

    QSpinBox *attendanceSpin = new QSpinBox();

    attendanceSpin->setRange(0, 100);

    attendanceSpin->setValue(75);

    attendanceSpin->setSuffix(" %");


    // Add fields to form

    formLayout->addRow(
        "Subject Code:",
        codeEdit
        );

    formLayout->addRow(
        "Subject Name:",
        nameEdit
        );

    formLayout->addRow(
        "Semester:",
        semesterCombo
        );

    formLayout->addRow(
        "Department:",
        departmentCombo
        );

    formLayout->addRow(
        "Min Attendance:",
        attendanceSpin
        );


    mainLayout->addLayout(formLayout);

    mainLayout->addStretch();


    // =========================
    // Buttons
    // =========================

    QHBoxLayout *buttonLayout =
        new QHBoxLayout();

    QPushButton *cancelButton =
        new QPushButton("Cancel");

    QPushButton *addButton =
        new QPushButton("Add Subject");


    cancelButton->setMinimumHeight(42);
    addButton->setMinimumHeight(42);


    buttonLayout->addStretch();

    buttonLayout->addWidget(
        cancelButton
        );

    buttonLayout->addWidget(
        addButton
        );


    mainLayout->addLayout(
        buttonLayout
        );


    // =========================
    // Connections
    // =========================

    connect(
        cancelButton,
        &QPushButton::clicked,
        &dialog,
        &QDialog::reject
        );


    connect(
        addButton,
        &QPushButton::clicked,
        [&dialog]() {

            dialog.accept();

        }
        );


    // =========================
    // Dialog Styling
    // =========================

    dialog.setStyleSheet(QString(R"(
QDialog {
    background:%1;
    color:%2;
}

QLabel {
    color:%2;
}

QLineEdit,
QComboBox,
QSpinBox {
    background:%3;
    border:1px solid %4;
    border-radius:10px;
    padding:8px 12px;
    color:%2;
    min-height:20px;
}

QLineEdit:focus,
QComboBox:focus,
QSpinBox:focus {
    border:1px solid %5;
}

QPushButton {
    background:%6;
    color:%2;
    border:1px solid %4;
    border-radius:10px;
    padding:8px 18px;
}

QPushButton:hover {
    background:%7;
}
)")
                             .arg(Theme::Surface)
                             .arg(Theme::Primary)
                             .arg(Theme::Input)
                             .arg(Theme::Border)
                             .arg(Theme::Gold)
                             .arg(Theme::Card)
                             .arg(Theme::Hover));


    // =========================
    // Show Dialog
    // =========================

    if (dialog.exec() == QDialog::Accepted) {

        // Temporary UI behavior.
        // DAO integration will be added later.

        QString code =
            codeEdit->text().trimmed();

        QString name =
            nameEdit->text().trimmed();

        QString semester =
            semesterCombo->currentText();

        QString department =
            departmentCombo->currentText();

        int minAttendance =
            attendanceSpin->value();


        // Temporary validation

        if (code.isEmpty() ||
            name.isEmpty() ||
            semesterCombo->currentIndex() == 0 ||
            departmentCombo->currentIndex() == 0) {

            QMessageBox::warning(
                this,
                "Incomplete Information",
                "Please fill in all subject details."
                );

            return;
        }


        // Temporary confirmation

        QMessageBox::information(
            this,
            "Subject Ready",
            "Subject information has been entered.\n\n"
            "Code: " + code +
                "\nName: " + name +
                "\n" + semester +
                "\nDepartment: " + department +
                "\nMinimum Attendance: " +
                QString::number(minAttendance) + "%"
            );
    }
}


// =====================================
// Edit Subject
// =====================================

void SubjectManagementWindow::onEditSubject()
{
    // =========================
    // Check Selection
    // =========================

    int selectedRow = subjectTable->currentRow();

    if (selectedRow < 0) {
        QMessageBox::warning(
            this,
            "No Selection",
            "Please select a subject to edit."
            );
        return;
    }


    // =========================
    // Get Current Subject Data
    // =========================

    QString currentCode =
        subjectTable->item(selectedRow, 0)->text();

    QString currentName =
        subjectTable->item(selectedRow, 1)->text();

    QString currentSemester =
        subjectTable->item(selectedRow, 2)->text();

    QString currentDepartment =
        subjectTable->item(selectedRow, 3)->text();

    int currentAttendance =
        subjectTable->item(selectedRow, 4)->text().toInt();


    // =========================
    // Create Dialog
    // =========================

    QDialog dialog(this);

    dialog.setWindowTitle("Edit Subject");
    dialog.setModal(true);
    dialog.resize(450, 400);


    QVBoxLayout *mainLayout =
        new QVBoxLayout(&dialog);


    // =========================
    // Title
    // =========================

    QLabel *titleLabel =
        new QLabel("EDIT SUBJECT");

    titleLabel->setFont(
        FontManager::headingFont(20)
        );

    titleLabel->setAlignment(
        Qt::AlignCenter
        );

    titleLabel->setStyleSheet(
        QString("color:%1;")
            .arg(Theme::Gold)
        );

    mainLayout->addWidget(
        titleLabel
        );

    mainLayout->addSpacing(15);


    // =========================
    // Form
    // =========================

    QFormLayout *formLayout =
        new QFormLayout();

    formLayout->setSpacing(15);


    // Subject Code

    QLineEdit *codeEdit =
        new QLineEdit();

    codeEdit->setText(
        currentCode
        );


    // Subject Name

    QLineEdit *nameEdit =
        new QLineEdit();

    nameEdit->setText(
        currentName
        );


    // Semester

    QComboBox *semesterCombo =
        new QComboBox();

    semesterCombo->addItem(
        "Select Semester"
        );

    for (int i = 1; i <= 8; ++i) {

        semesterCombo->addItem(
            QString("Semester %1").arg(i),
            i
            );
    }

    int semesterIndex =
        semesterCombo->findText(
            "Semester " + currentSemester
            );

    if (semesterIndex >= 0) {

        semesterCombo->setCurrentIndex(
            semesterIndex
            );
    }


    // Department

    QComboBox *departmentCombo =
        new QComboBox();

    departmentCombo->addItem(
        "Select Department"
        );

    departmentCombo->addItems({
        "Computer Engineering",
        "Civil Engineering",
        "Electrical Engineering",
        "Electronics Engineering",
        "Mechanical Engineering"
    });

    int departmentIndex =
        departmentCombo->findText(
            currentDepartment
            );

    if (departmentIndex >= 0) {

        departmentCombo->setCurrentIndex(
            departmentIndex
            );
    }


    // Minimum Attendance

    QSpinBox *attendanceSpin =
        new QSpinBox();

    attendanceSpin->setRange(
        0,
        100
        );

    attendanceSpin->setValue(
        currentAttendance
        );

    attendanceSpin->setSuffix(
        " %"
        );


    // =========================
    // Add Fields
    // =========================

    formLayout->addRow(
        "Subject Code:",
        codeEdit
        );

    formLayout->addRow(
        "Subject Name:",
        nameEdit
        );

    formLayout->addRow(
        "Semester:",
        semesterCombo
        );

    formLayout->addRow(
        "Department:",
        departmentCombo
        );

    formLayout->addRow(
        "Min Attendance:",
        attendanceSpin
        );


    mainLayout->addLayout(
        formLayout
        );

    mainLayout->addStretch();


    // =========================
    // Buttons
    // =========================

    QHBoxLayout *buttonLayout =
        new QHBoxLayout();

    QPushButton *cancelButton =
        new QPushButton("Cancel");

    QPushButton *saveButton =
        new QPushButton("Save Changes");


    cancelButton->setMinimumHeight(
        42
        );

    saveButton->setMinimumHeight(
        42
        );


    buttonLayout->addStretch();

    buttonLayout->addWidget(
        cancelButton
        );

    buttonLayout->addWidget(
        saveButton
        );


    mainLayout->addLayout(
        buttonLayout
        );


    // =========================
    // Button Connections
    // =========================

    connect(
        cancelButton,
        &QPushButton::clicked,
        &dialog,
        &QDialog::reject
        );


    connect(
        saveButton,
        &QPushButton::clicked,
        [&dialog]() {

            dialog.accept();

        }
        );


    // =========================
    // Dialog Styling
    // =========================

    dialog.setStyleSheet(QString(R"(
QDialog {
    background:%1;
    color:%2;
}

QLabel {
    color:%2;
}

QLineEdit,
QComboBox,
QSpinBox {
    background:%3;
    border:1px solid %4;
    border-radius:10px;
    padding:8px 12px;
    color:%2;
    min-height:20px;
}

QLineEdit:focus,
QComboBox:focus,
QSpinBox:focus {
    border:1px solid %5;
}

QPushButton {
    background:%6;
    color:%2;
    border:1px solid %4;
    border-radius:10px;
    padding:8px 18px;
}

QPushButton:hover {
    background:%7;
}
)")
                             .arg(Theme::Surface)
                             .arg(Theme::Primary)
                             .arg(Theme::Input)
                             .arg(Theme::Border)
                             .arg(Theme::Gold)
                             .arg(Theme::Card)
                             .arg(Theme::Hover));


    // =========================
    // Show Dialog
    // =========================

    if (dialog.exec() == QDialog::Accepted) {

        // Get Updated Values

        QString updatedCode =
            codeEdit->text().trimmed();

        QString updatedName =
            nameEdit->text().trimmed();

        QString updatedSemester =
            semesterCombo->currentText();

        QString updatedDepartment =
            departmentCombo->currentText();

        int updatedAttendance =
            attendanceSpin->value();


        // =========================
        // Temporary Validation
        // =========================

        if (updatedCode.isEmpty() ||
            updatedName.isEmpty() ||
            semesterCombo->currentIndex() == 0 ||
            departmentCombo->currentIndex() == 0) {

            QMessageBox::warning(
                this,
                "Incomplete Information",
                "Please fill in all subject details."
                );

            return;
        }


        // =========================
        // Temporary UI Update
        // =========================

        subjectTable->item(
                        selectedRow,
                        0
                        )->setText(
                updatedCode
                );

        subjectTable->item(
                        selectedRow,
                        1
                        )->setText(
                updatedName
                );

        subjectTable->item(
                        selectedRow,
                        2
                        )->setText(
                QString::number(
                    semesterCombo->currentData().toInt()
                    )
                );

        subjectTable->item(
                        selectedRow,
                        3
                        )->setText(
                updatedDepartment
                );

        subjectTable->item(
                        selectedRow,
                        4
                        )->setText(
                QString::number(
                    updatedAttendance
                    )
                );


        QMessageBox::information(
            this,
            "Subject Updated",
            "Subject information has been updated successfully."
            );
    }
}


// =====================================
// Delete Subject
// =====================================

void SubjectManagementWindow::onDeleteSubject()
{
    // =========================
    // Check Selection
    // =========================

    int selectedRow = subjectTable->currentRow();

    if (selectedRow < 0) {
        QMessageBox::warning(
            this,
            "No Selection",
            "Please select a subject to delete."
            );
        return;
    }


    // =========================
    // Get Subject Information
    // =========================

    QString subjectCode =
        subjectTable->item(selectedRow, 0)->text();

    QString subjectName =
        subjectTable->item(selectedRow, 1)->text();


    // =========================
    // Confirmation Dialog
    // =========================

    QMessageBox confirmationBox(this);

    confirmationBox.setWindowTitle(
        "Delete Subject"
        );

    confirmationBox.setIcon(
        QMessageBox::Warning
        );

    confirmationBox.setText(
        "Are you sure you want to delete this subject?"
        );

    confirmationBox.setInformativeText(
        "Subject Code: " + subjectCode +
        "\nSubject Name: " + subjectName
        );

    QPushButton *deleteButton =
        confirmationBox.addButton(
            "Delete",
            QMessageBox::DestructiveRole
            );

    confirmationBox.addButton(
        "Cancel",
        QMessageBox::RejectRole
        );


    // =========================
    // Style Confirmation Dialog
    // =========================

    confirmationBox.setStyleSheet(QString(R"(
QMessageBox {
    background:%1;
    color:%2;
}

QLabel {
    color:%2;
}

QPushButton {
    background:%3;
    color:%2;
    border:1px solid %4;
    border-radius:10px;
    padding:8px 20px;
    min-width:80px;
}

QPushButton:hover {
    background:%5;
}
)")
                                      .arg(Theme::Surface)
                                      .arg(Theme::Primary)
                                      .arg(Theme::Card)
                                      .arg(Theme::Border)
                                      .arg(Theme::Hover));


    // =========================
    // Show Confirmation
    // =========================

    confirmationBox.exec();


    // =========================
    // Check User Choice
    // =========================

    if (confirmationBox.clickedButton() == deleteButton) {

        // =========================
        // Temporary UI Delete
        // =========================

        subjectTable->removeRow(
            selectedRow
            );


        // =========================
        // Success Message
        // =========================

        QMessageBox::information(
            this,
            "Subject Deleted",
            "The subject has been deleted successfully."
            );
    }
}


// =====================================
// Search
// =====================================

void SubjectManagementWindow::onSearchSubject()
{
    QString searchText = searchEdit->text().trimmed();

    for (int row = 0; row < subjectTable->rowCount(); ++row) {

        bool matchFound = false;

        for (int column = 0;
             column < subjectTable->columnCount();
             ++column) {

            QTableWidgetItem *item =
                subjectTable->item(row, column);

            if (item &&
                item->text().contains(
                    searchText,
                    Qt::CaseInsensitive
                    )) {

                matchFound = true;
                break;
            }
        }

        subjectTable->setRowHidden(row, !matchFound);
    }
}