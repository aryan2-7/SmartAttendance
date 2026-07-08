#ifndef ADMINDASHBOARD_H
#define ADMINDASHBOARD_H

#include <QWidget>

class AdminDashboard : public QWidget
{
    Q_OBJECT

public:
    explicit AdminDashboard(QWidget *parent = nullptr);

private:
    void setupUI();
};

#endif // ADMINDASHBOARD_H