#pragma once

#include <QWidget>

class QLabel;
class QPushButton;

class WelcomeWindow : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomeWindow(QWidget *parent = nullptr);

private:
    void setupUI();
};