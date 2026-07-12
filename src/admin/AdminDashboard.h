#pragma once

#include <QWidget>

class QLabel;
class QFrame;
class CircularProgress;

class AdminDashboard : public QWidget
{
    Q_OBJECT

public:
    explicit AdminDashboard(QWidget *parent = nullptr);

private:
    void setupUI();
    void refreshDashboard();

    CircularProgress *progressCircle;
    QLabel *presentCount;
    QLabel *lateCount;
    QLabel *absentCount;
    QFrame *recentCard;
};
