#include "ManageStudentsWindow.h"
#include "../auth/FontManager.h"
#include "AdminDashboard.h"
#include "../theme/Theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>

ManageStudentsWindow::ManageStudentsWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void ManageStudentsWindow::setupUI()
{
    setWindowTitle("Manage Students");
    resize(1400,850);
    setObjectName("ManageStudentsWindow");

    setStyleSheet(QString(R"(

QWidget#ManageStudentsWindow{
    background:%1;
}

QFrame#card{
    background:%3;
    border:1px solid %2;
    border-radius:16px;
}

QPushButton{
    background:%3;
    color:%4;
    border:1px solid %2;
    border-radius:10px;
    padding:10px 18px;
}

QPushButton:hover{
    background:%5;
}

QLabel{
    border:none;
    background:transparent;
    color:%4;
}

QLineEdit{
    background:%6;
    color:%4;
    border:1px solid %2;
    border-radius:10px;
    padding:10px;
    font-size:14px;
}

QLineEdit:focus{
    border:1px solid %7;
}

QTableWidget{
    background:%6;
    border:1px solid %2;
    border-radius:14px;
    gridline-color:%2;
    selection-background-color:%7;
    color:%4;
    font-size:14px;
}

QHeaderView::section{
    background:%3;
    color:%4;
    border:none;
    padding:10px;
    font-weight:bold;
}

)")
                      .arg(Theme::Card)
                      .arg(Theme::Border)
                      .arg(Theme::Surface)
                      .arg(Theme::Primary)
                      .arg(Theme::Hover)
                      .arg(Theme::Input)
                      .arg(Theme::Gold));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30,25,30,25);
    mainLayout->setSpacing(25);
    mainLayout->setAlignment(Qt::AlignTop);

    //--------------------------------------------------
    // Header
    //--------------------------------------------------

    QHBoxLayout *header = new QHBoxLayout();

    QPushButton *backButton = new QPushButton("Back");
    connect(backButton, &QPushButton::clicked, this, [this]()
            {
                auto *window = new AdminDashboard();
                window->show();
                this->close();
            });
    backButton->setFixedSize(100,40);

    QLabel *title = new QLabel("Manage Students");
    title->setStyleSheet(QString("color:%1;").arg(Theme::Gold));

    header->addWidget(backButton);
    header->addSpacing(20);
    header->addWidget(title);
    header->addStretch();

    mainLayout->addLayout(header);

    //--------------------------------------------------
    // Card container (search bar + table live inside this,
    // styled to match the boxed "data" panel in the reference)
    //--------------------------------------------------

    QFrame *card = new QFrame();
    card->setObjectName("card");

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20,20,20,20);
    cardLayout->setSpacing(20);

    //--------------------------------------------------
    // Search Bar
    //--------------------------------------------------

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(15);

    QLineEdit *searchBox = new QLineEdit();
    searchBox->setPlaceholderText("Search Student...");
    searchBox->setFixedHeight(42);
    searchBox->setStyleSheet(QString(R"(
QLineEdit{
    background:%1;
    border:1px solid %2;
    color:%3;
}

QLineEdit::placeholder{
    color:%4;
}
)")
                                 .arg(Theme::Input)
                                 .arg(Theme::Border)
                                 .arg(Theme::Primary)
                                 .arg(Theme::Muted));

    QPushButton *refreshButton = new QPushButton("Refresh");
    refreshButton->setFixedSize(110,42);

    searchLayout->addWidget(searchBox);
    searchLayout->addWidget(refreshButton);

    cardLayout->addLayout(searchLayout);
    //--------------------------------------------------
    // Student Table
    //--------------------------------------------------

    QTableWidget *table = new QTableWidget();

    table->setColumnCount(5);

    table->setHorizontalHeaderLabels({
        "Student ID",
        "Student Name",
        "Roll No",
        "Edit",
        "Delete"
    });

    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    table->verticalHeader()->hide();

    table->setAlternatingRowColors(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    cardLayout->addWidget(table);

    // Card is fully built now — add it to the page
    mainLayout->addWidget(card);
    //--------------------------------------------------
    // Sample Data
    //--------------------------------------------------

    table->setRowCount(5);

    table->setItem(0,0,new QTableWidgetItem("1"));
    table->setItem(0,1,new QTableWidgetItem("Arhan Watson "));
    table->setItem(0,2,new QTableWidgetItem("15"));

    table->setItem(1,0,new QTableWidgetItem("2"));
    table->setItem(1,1,new QTableWidgetItem("Emma Wilson"));
    table->setItem(1,2,new QTableWidgetItem("22"));

    table->setItem(2,0,new QTableWidgetItem("3"));
    table->setItem(2,1,new QTableWidgetItem("John Smith"));
    table->setItem(2,2,new QTableWidgetItem("08"));

    table->setItem(3,0,new QTableWidgetItem("4"));
    table->setItem(3,1,new QTableWidgetItem("Sophia Brown"));
    table->setItem(3,2,new QTableWidgetItem("19"));

    table->setItem(4,0,new QTableWidgetItem("5"));
    table->setItem(4,1,new QTableWidgetItem("David Miller"));
    table->setItem(4,2,new QTableWidgetItem("11"));

    for(int row = 0; row < table->rowCount(); row++)
    {
        QTableWidgetItem *editItem = new QTableWidgetItem("Edit");
        editItem->setTextAlignment(Qt::AlignCenter);
        editItem->setForeground(QBrush(QColor(Theme::Gold)));

        QTableWidgetItem *deleteItem = new QTableWidgetItem("Delete");
        deleteItem->setTextAlignment(Qt::AlignCenter);
        deleteItem->setForeground(QBrush(QColor(Theme::Danger)));

        table->setItem(row,3,editItem);
        table->setItem(row,4,deleteItem);
    }
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    QLabel *totalStudents = new QLabel("Total Registered Students : 5");
    totalStudents->setStyleSheet(QString(
                                     "color:%1;"
                                     "font-size:14px;"
                                     )
                                     .arg(Theme::Secondary));

    mainLayout->addWidget(totalStudents);

    setLayout(mainLayout);
}