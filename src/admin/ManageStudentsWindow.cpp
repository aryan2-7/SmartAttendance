#include "ManageStudentsWindow.h"
#include "../auth/FontManager.h"
#include "AdminDashboard.h"
#include "../theme/Theme.h"
#include "../db/Database.h"
#include "../db/StudentDAO.h"
#include "../db/SubjectDAO.h"
#include "../db/DbPath.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDialog>
#include <QCheckBox>
#include <QScrollArea>
#include <QDialogButtonBox>
#include <set>

ManageStudentsWindow::ManageStudentsWindow(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setupUI();
    refreshTable();
}

void ManageStudentsWindow::refreshTable() {
    Database db(appDbPath());
    db.initializeTables();
    StudentDAO studentDAO(db.getConnection());
    std::vector<StudentRecord> students = studentDAO.getAllStudents();

    table->setRowCount(static_cast<int>(students.size()));
    for (size_t i = 0; i < students.size(); ++i) {
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(students[i].studentId));
        idItem->setData(Qt::UserRole, students[i].studentRollNumber);
        idItem->setData(Qt::UserRole + 1, QString::fromStdString(students[i].studentModelPath));
        table->setItem(i, 0, idItem);
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(students[i].studentName)));
        table->setItem(i, 2, new QTableWidgetItem(QString::number(students[i].studentRollNumber)));

        QTableWidgetItem *subjectsItem = new QTableWidgetItem("Manage");
        subjectsItem->setTextAlignment(Qt::AlignCenter);
        subjectsItem->setForeground(QBrush(QColor(Theme::Success)));
        table->setItem(i, 3, subjectsItem);

        QTableWidgetItem *editItem = new QTableWidgetItem("Edit");
        editItem->setTextAlignment(Qt::AlignCenter);
        editItem->setForeground(QBrush(QColor(Theme::Gold)));
        table->setItem(i, 4, editItem);

        QTableWidgetItem *deleteItem = new QTableWidgetItem("Delete");
        deleteItem->setTextAlignment(Qt::AlignCenter);
        deleteItem->setForeground(QBrush(QColor(Theme::Danger)));
        table->setItem(i, 5, deleteItem);
    }

    totalStudentsLabel->setText(
        QString("Total Registered Students : %1").arg(students.size()));
}

void ManageStudentsWindow::setupUI()
{
    setWindowTitle("Manage Students");
    resize(1400,850); // fallback size if shown without maximizing
    setMinimumSize(1300, 800);
    setObjectName("ManageStudentsWindow");

    setStyleSheet(QString(R"(
QWidget#ManageStudentsWindow{
    background:%1;
}
QFrame#card{
    background:%3;
    border:1px solid %2;
    border-radius:16px;
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
QLabel{
    border:none;
    background:transparent;
    color:%4;
}
QLineEdit{
    background:%6;
    color:%4;
    border:1px solid %2;
    border-radius:10px;
    padding:10px;
    font-size:14px;
}
QLineEdit:focus{
    border:1px solid %7;
}
QTableWidget{
    background:%6;
    border:1px solid %2;
    border-radius:14px;
    gridline-color:%2;
    selection-background-color:%7;
    color:%4;
    font-size:14px;
}
QHeaderView::section{
    background:%3;
    color:%4;
    border:none;
    padding:10px;
    font-weight:bold;
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
                window->showMaximized();
                this->close();
            });
    backButton->setFixedSize(100,40);

    QLabel *title = new QLabel("Manage Students");
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

    // Search
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(15);

    QLineEdit *searchBox = new QLineEdit();
    searchBox->setPlaceholderText("Search Student...");
    searchBox->setFixedHeight(42);
    searchBox->setMinimumWidth(300);
    searchBox->setStyleSheet(QString(R"(
QLineEdit{
    background:%1;
    border:1px solid %2;
    color:%3;
}
QLineEdit::placeholder{
    color:%4;
}
)")
                                 .arg(Theme::Input)
                                 .arg(Theme::Border)
                                 .arg(Theme::Primary)
                                 .arg(Theme::Muted));

    QPushButton *refreshButton = new QPushButton("Refresh");
    refreshButton->setFixedSize(110,42);
    connect(refreshButton, &QPushButton::clicked, this, &ManageStudentsWindow::refreshTable);

    searchLayout->addWidget(searchBox);
    searchLayout->addWidget(refreshButton);
    cardLayout->addLayout(searchLayout);

    // Table
    table = new QTableWidget();
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({
        "Student ID", "Student Name", "Roll No",
        "Subjects", "Edit", "Delete"
    });
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->hide();
    table->setAlternatingRowColors(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cardLayout->addWidget(table);
    mainLayout->addWidget(card);

    totalStudentsLabel = new QLabel("Total Registered Students : 0");
    totalStudentsLabel->setStyleSheet(QString("color:%1; font-size:14px;")
                                          .arg(Theme::Secondary));
    mainLayout->addWidget(totalStudentsLabel);

    setLayout(mainLayout);

    // Wire Edit/Delete via cell click
    connect(table, &QTableWidget::cellClicked, this, [this](int row, int col) {
        if (row < 0 || row >= table->rowCount()) return;

        int studentId = table->item(row, 0)->text().toInt();
        int rollNumber = table->item(row, 0)->data(Qt::UserRole).toInt();

        if (col == 3) { // Subjects (enroll / unenroll)
            openEnrollmentDialog(studentId, table->item(row, 1)->text());

        } else if (col == 4) { // Edit
            QString newName = QInputDialog::getText(
                this, "Edit Student", "New Name:",
                QLineEdit::Normal, table->item(row, 1)->text());
            if (newName.isEmpty()) return;

            int newRoll = QInputDialog::getInt(
                this, "Edit Student", "New Roll Number:",
                rollNumber, 1, 99999);
            if (newRoll <= 0) return;

            Database db(appDbPath());
            db.initializeTables();
            StudentDAO studentDAO(db.getConnection());

            if (newRoll != rollNumber && studentDAO.studentExists(newRoll)) {
                QMessageBox::warning(this, "Duplicate Roll",
                    "Another student already has roll number " + QString::number(newRoll) + ".");
                return;
            }

            QString oldPath = table->item(row, 0)->data(Qt::UserRole + 1).toString();
            QString newPath;
            if (!oldPath.isEmpty()) {
                QFileInfo fi(oldPath);
                QString dir = fi.absolutePath();
                QString safeName = newName;
                safeName.replace(' ', '_');
                newPath = dir + "/" + safeName + "_" + QString::number(newRoll) + ".bin";
                if (oldPath != newPath) {
                    QFile::rename(oldPath, newPath);
                }
            }

            if (!studentDAO.updateStudent(rollNumber, newName.toStdString(), newRoll, newPath.toStdString())) {
                QMessageBox::critical(this, "Error", "Failed to update student in database.");
                return;
            }
            refreshTable();

        } else if (col == 5) { // Delete
            auto reply = QMessageBox::question(
                this, "Confirm Delete",
                "Are you sure you want to delete " +
                table->item(row, 1)->text() + "?",
                QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                QString modelPath = table->item(row, 0)->data(Qt::UserRole + 1).toString();
                if (!modelPath.isEmpty()) {
                    QFile file(modelPath);
                    if (file.exists()) {
                        file.remove();
                    }
                }
                Database db(appDbPath());
                db.initializeTables();
                StudentDAO studentDAO(db.getConnection());
                if (!studentDAO.deleteStudent(rollNumber)) {
                    QMessageBox::critical(this, "Error", "Failed to delete student from database.");
                    return;
                }
                refreshTable();
            }
        }
    });
}

// =====================================
// Enrollment dialog: pick which subjects this student is enrolled in
// =====================================

void ManageStudentsWindow::openEnrollmentDialog(int studentId, const QString &studentName) {
    Database db(appDbPath());
    db.initializeTables();
    SubjectDAO subjectDAO(db.getConnection());
    StudentDAO studentDAO(db.getConnection());

    std::vector<SubjectRecord> subjects = subjectDAO.getAllSubjects();
    std::vector<EnrollmentRecord> enrollments = studentDAO.getEnrollmentsForStudent(studentId);

    std::set<int> enrolledSubjectIds;
    for (auto &e : enrollments) enrolledSubjectIds.insert(e.enrollmentSubjectId);

    QDialog dialog(this);
    dialog.setWindowTitle("Manage Subjects — " + studentName);
    dialog.resize(420, 480);
    dialog.setStyleSheet(styleSheet());

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *header = new QLabel("Select the subjects " + studentName + " is enrolled in:");
    header->setWordWrap(true);
    layout->addWidget(header);

    if (subjects.empty()) {
        layout->addWidget(new QLabel(
            "No subjects exist yet. Add some from Subject Management first."));
    }

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    QWidget *checklistWidget = new QWidget();
    QVBoxLayout *checklistLayout = new QVBoxLayout(checklistWidget);

    std::vector<QCheckBox*> checkBoxes;
    std::vector<int> subjectIds;
    for (auto &s : subjects) {
        QCheckBox *cb = new QCheckBox(
            QString::fromStdString(s.subjectCode + " - " + s.subjectName));
        cb->setChecked(enrolledSubjectIds.count(s.subjectId) > 0);
        checklistLayout->addWidget(cb);
        checkBoxes.push_back(cb);
        subjectIds.push_back(s.subjectId);
    }
    checklistLayout->addStretch();
    scrollArea->setWidget(checklistWidget);
    layout->addWidget(scrollArea);

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    int changed = 0;
    for (size_t i = 0; i < checkBoxes.size(); ++i) {
        bool wasEnrolled = enrolledSubjectIds.count(subjectIds[i]) > 0;
        bool nowChecked = checkBoxes[i]->isChecked();
        if (nowChecked && !wasEnrolled) {
            if (studentDAO.enrollStudent(studentId, subjectIds[i])) ++changed;
        } else if (!nowChecked && wasEnrolled) {
            if (studentDAO.unenrollStudent(studentId, subjectIds[i])) ++changed;
        }
    }

    if (changed > 0) {
        QMessageBox::information(this, "Subjects Updated",
            QString("Updated %1 enrollment(s) for %2.").arg(changed).arg(studentName));
    }
}
