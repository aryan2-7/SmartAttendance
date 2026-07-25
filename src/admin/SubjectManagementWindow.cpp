#include "SubjectManagementWindow.h"
#include "../db/SubjectDAO.h"
#include "../theme/Theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QFont>

SubjectManagementWindow::SubjectManagementWindow(QWidget *parent)
    : QWidget(parent), dbConn("") {
    dbConn = Database(std::string(PROJECT_SOURCE_DIR) + "/smart_attendance.db");
    dbConn.initializeTables();
    setupUI();
    refreshTable();
}

void SubjectManagementWindow::setupUI() {
    setStyleSheet(QString("QWidget{ background:%1; color:%2; }")
                      .arg(Theme::Card).arg(Theme::Primary));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    QLabel *title = new QLabel("Subject Management");
    QFont titleFont;
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setStyleSheet(QString("color:%1; padding-bottom:8px;").arg(Theme::Gold));
    mainLayout->addWidget(title);

    // Form group
    QGroupBox *formGroup = new QGroupBox("Subject Details");
    formGroup->setStyleSheet(QString(
        "QGroupBox{ background:%1; border:1px solid %2; border-radius:8px; "
        "margin-top:16px; padding-top:20px; font-weight:bold; color:%3; }"
        "QGroupBox::title{ subcontrol-origin:margin; left:12px; padding:0 4px; }")
        .arg(Theme::Surface).arg(Theme::Border).arg(Theme::Primary));

    QFormLayout *formLayout = new QFormLayout(formGroup);
    formLayout->setSpacing(10);
    formLayout->setContentsMargins(16, 24, 16, 16);

    codeEdit = new QLineEdit();
    codeEdit->setPlaceholderText("e.g. ENGG102");
    codeEdit->setStyleSheet(QString("QLineEdit{ background:%1; color:%2; border:1px solid %3; "
                                    "border-radius:6px; padding:6px; }")
                            .arg(Theme::Input).arg(Theme::Primary).arg(Theme::Border));
    formLayout->addRow("Subject Code:", codeEdit);

    nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("e.g. Engineering Mathematics");
    nameEdit->setStyleSheet(codeEdit->styleSheet());
    formLayout->addRow("Subject Name:", nameEdit);

    semesterSpin = new QSpinBox();
    semesterSpin->setRange(1, 8);
    semesterSpin->setStyleSheet(QString("QSpinBox{ background:%1; color:%2; border:1px solid %3; "
                                        "border-radius:6px; padding:6px; }")
                                .arg(Theme::Input).arg(Theme::Primary).arg(Theme::Border));
    formLayout->addRow("Semester:", semesterSpin);

    deptCombo = new QComboBox();
    deptCombo->addItems({"Computer Science", "Electrical", "Mechanical", "Civil", "Electronics", "Other"});
    deptCombo->setStyleSheet(QString("QComboBox{ background:%1; color:%2; border:1px solid %3; "
                                     "border-radius:6px; padding:6px; }")
                             .arg(Theme::Input).arg(Theme::Primary).arg(Theme::Border));
    formLayout->addRow("Department:", deptCombo);

    minAttSpin = new QSpinBox();
    minAttSpin->setRange(0, 100);
    minAttSpin->setValue(80);
    minAttSpin->setSuffix("%");
    minAttSpin->setStyleSheet(semesterSpin->styleSheet());
    formLayout->addRow("Min Attendance:", minAttSpin);

    mainLayout->addWidget(formGroup);

    // Action buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    addBtn = new QPushButton("Add Subject");
    editBtn = new QPushButton("Update Subject");
    deleteBtn = new QPushButton("Delete Subject");

    QString btnStyle = QString(
        "QPushButton{ background:%1; color:%2; border:1px solid %3; "
        "border-radius:8px; padding:10px 20px; font-weight:bold; }"
        "QPushButton:hover{ background:%4; }")
        .arg(Theme::Surface).arg(Theme::Primary).arg(Theme::Border).arg(Theme::Hover);

    addBtn->setStyleSheet(btnStyle);
    editBtn->setStyleSheet(btnStyle);
    deleteBtn->setStyleSheet(btnStyle);

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(editBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    connect(addBtn, &QPushButton::clicked, this, &SubjectManagementWindow::onAddClicked);
    connect(editBtn, &QPushButton::clicked, this, &SubjectManagementWindow::onEditClicked);
    connect(deleteBtn, &QPushButton::clicked, this, &SubjectManagementWindow::onDeleteClicked);

    // Status label
    statusLabel = new QLabel();
    statusLabel->setStyleSheet(QString("QLabel{ color:%1; padding:4px; }").arg(Theme::Success));
    mainLayout->addWidget(statusLabel);

    // Table
    subjectTable = new QTableWidget();
    subjectTable->setColumnCount(6);
    subjectTable->setHorizontalHeaderLabels(
        {"ID", "Code", "Name", "Semester", "Department", "Min %"});
    subjectTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    subjectTable->setSelectionMode(QAbstractItemView::SingleSelection);
    subjectTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    subjectTable->horizontalHeader()->setStretchLastSection(true);
    subjectTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    subjectTable->verticalHeader()->hide();
    subjectTable->setStyleSheet(QString(
        "QTableWidget{ background:%1; color:%2; border:1px solid %3; "
        "border-radius:6px; gridline-color:%4; }"
        "QTableWidget::item{ padding:6px; }"
        "QHeaderView::section{ background:%5; color:%2; border:1px solid %3; padding:6px; font-weight:bold; }")
        .arg(Theme::Input).arg(Theme::Primary).arg(Theme::Border)
        .arg(Theme::Border).arg(Theme::Surface));

    mainLayout->addWidget(subjectTable);

    connect(subjectTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        int row = subjectTable->currentRow();
        if (row < 0) return;
        codeEdit->setText(subjectTable->item(row, 1)->text());
        nameEdit->setText(subjectTable->item(row, 2)->text());
        semesterSpin->setValue(subjectTable->item(row, 3)->text().toInt());
        deptCombo->setCurrentText(subjectTable->item(row, 4)->text());
        minAttSpin->setValue(subjectTable->item(row, 5)->text().replace("%","").toInt());
    });
}

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
