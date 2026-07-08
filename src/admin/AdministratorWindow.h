#ifndef ADMINISTRATORWINDOW_H
#define ADMINISTRATORWINDOW_H

#include <QWidget>

class QLineEdit;
class QPushButton;

class AdministratorWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AdministratorWindow(QWidget *parent = nullptr);

private:
    void setupUI();

    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QPushButton *backButton;
};

#endif // ADMINISTRATORWINDOW_H