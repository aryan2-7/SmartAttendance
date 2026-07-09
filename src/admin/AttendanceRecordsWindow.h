#ifndef ATTENDANCERECORDSWINDOW_H
#define ATTENDANCERECORDSWINDOW_H

#include <QWidget>

class AttendanceRecordsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AttendanceRecordsWindow(QWidget *parent = nullptr);

private:
    void setupUI();
};

#endif // ATTENDANCERECORDSWINDOW_H