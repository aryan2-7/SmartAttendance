#pragma once

#include <QWidget>

class QTableWidget;
class QLabel;

class ManageStudentsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ManageStudentsWindow(QWidget *parent = nullptr);

private:
    void setupUI();
    void refreshTable();

    QTableWidget *table;
    QLabel *totalStudentsLabel;
};
