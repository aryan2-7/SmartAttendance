#pragma once

#include <QWidget>

class QComboBox;
class QTableWidget;
class QLabel;

class AttendanceRecordsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AttendanceRecordsWindow(QWidget *parent = nullptr);

private slots:
    void refreshView();

private:
    void setupUI();

    QComboBox *subjectCombo;
    QComboBox *rangeCombo;
    QTableWidget *rosterTable;
    QTableWidget *percentTable;
    QLabel *rangeSummaryLabel;
};
