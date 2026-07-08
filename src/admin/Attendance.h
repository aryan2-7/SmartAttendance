#ifndef ATTENDANCE_H
#define ATTENDANCE_H

#include <QWidget>

class Attendance: public QWidget
{
    Q_OBJECT

public:
    explicit Attendance(QWidget *parent = nullptr);

    void setValues(int present,
                   int late,
                   int absent);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    int present;
    int late;
    int absent;
};

#endif