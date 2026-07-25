#ifndef SUBJECTMANAGEMENTWINDOW_H
#define SUBJECTMANAGEMENTWINDOW_H

#include <QWidget>

class QTableWidget;
class QLineEdit;
class QPushButton;

class SubjectManagementWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SubjectManagementWindow(QWidget *parent = nullptr);

private slots:
    void onAddSubject();
    void onEditSubject();
    void onDeleteSubject();
    void onSearchSubject();

private:
    void setupUI();
    void setupStyles();
    void loadSubjects();

    QTableWidget *subjectTable;

    QLineEdit *searchEdit;

    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *backButton;
};

#endif // SUBJECTMANAGEMENTWINDOW_H