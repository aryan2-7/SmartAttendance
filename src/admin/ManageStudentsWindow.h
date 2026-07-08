#ifndef MANAGESTUDENTSWINDOW_H
#define MANAGESTUDENTSWINDOW_H

#include <QWidget>

class ManageStudentsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ManageStudentsWindow(QWidget *parent = nullptr);

private:
    void setupUI();
};

#endif // MANAGESTUDENTSWINDOW_H