#ifndef STUDENTREGISTRATIONWINDOW_H
#define STUDENTREGISTRATIONWINDOW_H

#include <QWidget>

class StudentRegistrationWindow : public QWidget
{
    Q_OBJECT

public:
    explicit StudentRegistrationWindow(QWidget *parent = nullptr);

private:
    void setupUI();
};

#endif // STUDENTREGISTRATIONWINDOW_H