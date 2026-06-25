#pragma once

#include <QWidget>

class MainMenu : public QWidget {
    Q_OBJECT
public:
    explicit MainMenu(QWidget *parent = nullptr);

private slots:
    void onRegisterClicked();
    void onMarkAttendanceClicked();
};
