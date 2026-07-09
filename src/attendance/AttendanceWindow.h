#ifndef ATTENDANCEWINDOW_H
#define ATTENDANCEWINDOW_H

#include <QWidget>

class AttendanceWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AttendanceWindow(QWidget *parent = nullptr);

private:
    void setupUI();
};

#endif // ATTENDANCEWINDOW_H