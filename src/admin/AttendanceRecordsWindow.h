#pragma once

#include <QWidget>

class AttendanceRecordsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AttendanceRecordsWindow(QWidget *parent = nullptr);

private:
    void setupUI();
};