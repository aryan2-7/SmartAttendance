#pragma once

#include <QWidget>

class QPushButton;
class QComboBox;
class QDateEdit;
class QTimeEdit;
class QLineEdit;
class QTableWidget;

class ScheduleEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ScheduleEditor(QWidget *parent = nullptr);

private:
    void setupUI();
    void setupStyles();
    void loadSessions();

private slots:
    void onAddSession();
    void onAddWeekly();
    void onDeleteSession();

private:
    QPushButton *backButton;

    QComboBox *subjectCombo;

    QDateEdit *dateEdit;

    QTimeEdit *startTimeEdit;
    QTimeEdit *endTimeEdit;

    QLineEdit *roomEdit;
    QLineEdit *topicEdit;

    QPushButton *addSessionButton;
    QPushButton *addWeeklyButton;

    QTableWidget *sessionTable;
    QPushButton *deleteButton;
};