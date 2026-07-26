#include "SubjectManagementWindow.h"
#include "../db/SubjectDAO.h"
#include "../db/DbPath.h"
#include "../theme/Theme.h"
#include "../auth/FontManager.h"
#include "AdminDashboard.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QComboBox>
#include <QSpinBox>
#include <QFont>
#include <QGroupBox>

SubjectManagementWindow::SubjectManagementWindow(QWidget *parent)
    : QWidget(parent), dbConn("") {
    dbConn = Database(appDbPath());
    dbConn.initializeTables();

    setupUI();
    setupStyles();
    refreshTable();
}

// =====================================
// Setup UI
// =====================================

void SubjectManagementWindow::setupUI() {
    setWindowTitle("Subject Management");
    resize(1200, 750); // fallback size if shown without maximizing

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
    titleLabel->setFont(FontManager::headingFont(22));

    headerLayout->addWidget(backButton);
    headerLayout->addStretch();
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    mainLayout->addLayout(headerLayout);

    // =========================
    // Subject Details Card
    // =========================

    QGroupBox *formGroup = new QGroupBox("Subject Details");
    formGroup->setObjectName("formGroup");

    QFormLayout *formLayout = new QFormLayout(formGroup);
    formLayout->setContentsMargins(25, 25, 25, 25);
    formLayout->setSpacing(15);

    codeEdit = new QLineEdit();
    codeEdit->setPlaceholderText("e.g. ENGG102");
    formLayout->addRow("Subject Code:", codeEdit);

    nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("e.g. Engineering Mathematics");
    formLayout->addRow("Subject Name:", nameEdit);

    semesterSpin = new QSpinBox();
    semesterSpin->setRange(1, 8);
    formLayout->addRow("Semester:", semesterSpin);

    deptCombo = new QComboBox();
    deptCombo->addItems({"Computer Engineering", "Electrical", "Mechanical", "Civil", "Electronics", "Other"});
    deptCombo->setMinimumWidth(200);
    formLayout->addRow("Department:", deptCombo);

    minAttSpin = new QSpinBox();
    minAttSpin->setRange(0, 100);
    minAttSpin->setValue(80);
    minAttSpin->setSuffix("%");
    formLayout->addRow("Min Attendance:", minAttSpin);

    mainLayout->addWidget(formGroup);

    // =========================
    // Action Buttons
    // =========================

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    addBtn = new QPushButton("Add Subject");
    editBtn = new QPushButton("Update Subject");
    deleteBtn = new QPushButton("Delete Subject");

    addBtn->setFixedHeight(45);
    editBtn->setFixedHeight(45);
    deleteBtn->setFixedHeight(45);

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(editBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addStretch();

    mainLayout->addLayout(btnLayout);

    connect(addBtn, &QPushButton::clicked, this, &SubjectManagementWindow::onAddClicked);
    connect(editBtn, &QPushButton::clicked, this, &SubjectManagementWindow::onEditClicked);
    connect(deleteBtn, &QPushButton::clicked, this, &SubjectManagementWindow::onDeleteClicked);

    // =========================
    // Status Label
    // =========================

    statusLabel = new QLabel();
    mainLayout->addWidget(statusLabel);

    // =========================
    // Search
    // =========================

    QHBoxLayout *searchLayout = new QHBoxLayout();

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Search subjects...");
    searchEdit->setFixedHeight(45);
    searchEdit->setMinimumWidth(300);

    searchLayout->addWidget(searchEdit);

    mainLayout->addLayout(searchLayout);

    connect(searchEdit, &QLineEdit::textChanged,
            this, &SubjectManagementWindow::onSearchSubject);

    // =========================
    // Subject Table
    // =========================

    subjectTable = new QTableWidget();
    subjectTable->setColumnCount(6);
    subjectTable->setHorizontalHeaderLabels(
        {"ID", "Code", "Name", "Semester", "Department", "Min %"});

    subjectTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    subjectTable->setSelectionMode(QAbstractItemView::SingleSelection);
    subjectTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    subjectTable->setAlternatingRowColors(false);
    subjectTable->verticalHeader()->setVisible(false);
    subjectTable->horizontalHeader()->setStretchLastSection(true);
    subjectTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    mainLayout->addWidget(subjectTable);

    connect(subjectTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        int row = subjectTable->currentRow();
        if (row < 0) return;
        codeEdit->setText(subjectTable->item(row, 1)->text());
        nameEdit->setText(subjectTable->item(row, 2)->text());
        semesterSpin->setValue(subjectTable->item(row, 3)->text().toInt());
        deptCombo->setCurrentText(subjectTable->item(row, 4)->text());
        minAttSpin->setValue(subjectTable->item(row, 5)->text().replace("%", "").toInt());
    });

    // =========================
    // Connections
    // =========================

    connect(backButton, &QPushButton::clicked, this, [this]() {
        auto *dashboard = new AdminDashboard();
        dashboard->showMaximized();
        this->close();
    });
}

// =====================================
// Styling
// =====================================

void SubjectManagementWindow::setupStyles() {
    setStyleSheet(QString(R"(
        QWidget {
            background:%1;
            color:%2;
        }

        QLabel {
            color:%2;
        }

        QGroupBox#formGroup {
            background:%3;
            border:1px solid %4;
            border-radius:18px;
            margin-top:12px;
            font-weight:bold;
            color:%2;
        }

        QGroupBox#formGroup::title {
            subcontrol-origin:margin;
            left:16px;
            padding:0 6px;
            color:%6;
        }

        QLineEdit,
        QComboBox,
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
        QSpinBox:focus {
            border:1px solid %6;
        }

        QComboBox QAbstractItemView {
            background:%5;
            border:1px solid %4;
            border-radius:6px;
            color:%2;
            min-width:350px;
            selection-background-color:%3;
            selection-color:%2;
            padding:4px;
        }

        QComboBox QAbstractItemView::item {
            padding:6px 12px;
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

        QPushButton:pressed {
            background:%4;
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

        QScrollBar:vertical {
            background:%5;
            width:10px;
            border-radius:5px;
        }

        QScrollBar::handle:vertical {
            background:%4;
            border-radius:5px;
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

void SubjectManagementWindow::refreshTable() {
    SubjectDAO subjectDAO(dbConn.getConnection());
    std::vector<SubjectRecord> subjects = subjectDAO.getAllSubjects();

    subjectTable->setRowCount(static_cast<int>(subjects.size()));
    for (int i = 0; i < static_cast<int>(subjects.size()); ++i) {
        auto &s = subjects[i];
        subjectTable->setItem(i, 0, new QTableWidgetItem(QString::number(s.subjectId)));
        subjectTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(s.subjectCode)));
        subjectTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(s.subjectName)));
        subjectTable->setItem(i, 3, new QTableWidgetItem(QString::number(s.subjectSemester)));
        subjectTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(s.subjectDepartment)));
        subjectTable->setItem(i, 5, new QTableWidgetItem(QString::number(s.subjectMinAttendance) + "%"));
    }
}

bool SubjectManagementWindow::validateInput(const std::string &code, const std::string &name,
                                            int semester, const std::string &dept, int minAtt) {
    if (code.empty()) { statusLabel->setText("Error: Subject code is required."); return false; }
    if (code.length() > 20) { statusLabel->setText("Error: Subject code too long (max 20 chars)."); return false; }
    if (name.empty()) { statusLabel->setText("Error: Subject name is required."); return false; }
    if (semester < 1 || semester > 8) { statusLabel->setText("Error: Semester must be 1-8."); return false; }
    if (dept.empty()) { statusLabel->setText("Error: Department is required."); return false; }
    if (minAtt < 0 || minAtt > 100) { statusLabel->setText("Error: Min attendance must be 0-100."); return false; }
    return true;
}

int SubjectManagementWindow::getSelectedSubjectId() {
    int row = subjectTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Please select a subject from the table first.");
        return -1;
    }
    return subjectTable->item(row, 0)->text().toInt();
}

void SubjectManagementWindow::clearForm() {
    codeEdit->clear();
    nameEdit->clear();
    semesterSpin->setValue(1);
    deptCombo->setCurrentIndex(0);
    minAttSpin->setValue(80);
    subjectTable->clearSelection();
}

void SubjectManagementWindow::onAddClicked() {
    std::string code = codeEdit->text().trimmed().toUpper().toStdString();
    std::string name = nameEdit->text().trimmed().toStdString();
    int semester = semesterSpin->value();
    std::string dept = deptCombo->currentText().toStdString();
    int minAtt = minAttSpin->value();

    if (!validateInput(code, name, semester, dept, minAtt)) return;

    SubjectDAO subjectDAO(dbConn.getConnection());

    SubjectRecord existing = subjectDAO.getSubjectByCode(code);
    if (existing.subjectId >= 0) {
        statusLabel->setText("Error: Subject code already exists.");
        return;
    }

    if (subjectDAO.createSubject(code, name, semester, dept, minAtt)) {
        statusLabel->setText("Subject added successfully.");
        refreshTable();
        clearForm();
        emit subjectChanged();
    } else {
        statusLabel->setText("Error: Failed to add subject.");
    }
}

void SubjectManagementWindow::onEditClicked() {
    int subjectId = getSelectedSubjectId();
    if (subjectId < 0) return;

    std::string code = codeEdit->text().trimmed().toUpper().toStdString();
    std::string name = nameEdit->text().trimmed().toStdString();
    int semester = semesterSpin->value();
    std::string dept = deptCombo->currentText().toStdString();
    int minAtt = minAttSpin->value();

    if (!validateInput(code, name, semester, dept, minAtt)) return;

    SubjectDAO subjectDAO(dbConn.getConnection());

    SubjectRecord existing = subjectDAO.getSubjectByCode(code);
    if (existing.subjectId >= 0 && existing.subjectId != subjectId) {
        statusLabel->setText("Error: Another subject already uses this code.");
        return;
    }

    if (subjectDAO.updateSubject(subjectId, code, name, semester, dept, minAtt)) {
        statusLabel->setText("Subject updated successfully.");
        refreshTable();
        emit subjectChanged();
    } else {
        statusLabel->setText("Error: Failed to update subject.");
    }
}

void SubjectManagementWindow::onDeleteClicked() {
    int subjectId = getSelectedSubjectId();
    if (subjectId < 0) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete",
        "Are you sure you want to delete this subject?\n"
        "This will also delete all related sessions and attendance records.",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    SubjectDAO subjectDAO(dbConn.getConnection());
    if (subjectDAO.deleteSubject(subjectId)) {
        statusLabel->setText("Subject deleted successfully.");
        refreshTable();
        clearForm();
        emit subjectChanged();
    } else {
        statusLabel->setText("Error: Failed to delete subject.");
    }
}

// =====================================
// Search (ported from dilasha, works over live DB-backed table)
// =====================================

void SubjectManagementWindow::onSearchSubject() {
    QString searchText = searchEdit->text().trimmed();

    for (int row = 0; row < subjectTable->rowCount(); ++row) {
        bool matchFound = false;

        for (int column = 0; column < subjectTable->columnCount(); ++column) {
            QTableWidgetItem *item = subjectTable->item(row, column);

            if (item && item->text().contains(searchText, Qt::CaseInsensitive)) {
                matchFound = true;
                break;
            }
        }

        subjectTable->setRowHidden(row, !matchFound);
    }
}