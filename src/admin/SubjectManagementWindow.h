#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include "../db/Database.h"

class SubjectManagementWindow : public QWidget {
    Q_OBJECT

public:
    explicit SubjectManagementWindow(QWidget *parent = nullptr);

signals:
    void subjectChanged();

public slots:
    void refreshTable();

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();

private:
    void setupUI();
    void clearForm();
    bool validateInput(const std::string &code, const std::string &name,
                       int semester, const std::string &dept, int minAtt);
    int  getSelectedSubjectId();

    QTableWidget *subjectTable;
    QLineEdit    *codeEdit;
    QLineEdit    *nameEdit;
    QSpinBox     *semesterSpin;
    QComboBox    *deptCombo;
    QSpinBox     *minAttSpin;
    QPushButton  *addBtn;
    QPushButton  *editBtn;
    QPushButton  *deleteBtn;
    QLabel       *statusLabel;

    Database dbConn;
};
