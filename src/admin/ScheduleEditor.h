#pragma once
#include <QWidget>
#include <QComboBox>
#include <QTableWidget>
#include <QDateEdit>
#include <QTimeEdit>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "../db/Database.h"

class ScheduleEditor : public QWidget {
    Q_OBJECT
public:
    explicit ScheduleEditor(QWidget *parent = nullptr);

signals:
    void scheduleChanged();

public slots:
    void refreshSessionList();

private slots:
    void onSubjectSelected(int index);
    void onAddSessionClicked();
    void onAddWeeklyClicked();
    void onDeleteSessionClicked();

private:
    void setupUI();
    void setupStyles();
    void addSingleSession(int subjectId, const std::string &date,
                          const std::string &start, const std::string &end,
                          const std::string &room, const std::string &topic);

    QPushButton   *backButton;
    QComboBox     *subjectCombo;
    QDateEdit     *dateEdit;
    QTimeEdit     *startTimeEdit;
    QTimeEdit     *endTimeEdit;
    QLineEdit     *roomEdit;
    QLineEdit     *topicEdit;
    QSpinBox      *weeksSpin;
    QPushButton   *addSessionBtn;
    QPushButton   *addWeeklyBtn;
    QPushButton   *deleteBtn;
    QTableWidget  *sessionTable;
    QLabel        *statusLabel;
    Database dbConn;
};