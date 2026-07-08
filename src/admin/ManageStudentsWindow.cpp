#include "ManageStudentsWindow.h"
#include "../auth/FontManager.h"
#include "AdminDashboard.h"

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

    setStyleSheet(R"(
QWidget{
    background:#0F172A;
    color:white;
}

QPushButton{
    background:#1E293B;
    color:white;
    border:1px solid #334155;
    border-radius:10px;
    padding:10px 18px;
}

QPushButton:hover{
    background:#334155;
}

QLabel{
    border:none;
    background:transparent;
}
QLineEdit{
    background:#111827;
    color:white;
    border:1px solid #334155;
    border-radius:10px;
    padding:10px;
    font-size:14px;
}

QLineEdit:focus{
    border:1px solid #2563EB;
}
QTableWidget{
    background:#111827;
    border:1px solid #334155;
    border-radius:14px;
    gridline-color:#334155;
    selection-background-color:#2563EB;
    color:white;
    font-size:14px;
}

QHeaderView::section{
    background:#1E293B;
    color:white;
    border:none;
    padding:10px;
    font-weight:bold;
}
)");

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
    title->setFont(FontManager::headingFont(26));

    header->addWidget(backButton);
    header->addSpacing(20);
    header->addWidget(title);
    header->addStretch();

    mainLayout->addLayout(header);
    //--------------------------------------------------
    // Search Bar
    //--------------------------------------------------

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(15);

    QLineEdit *searchBox = new QLineEdit();
    searchBox->setPlaceholderText("Search Student...");
    searchBox->setFixedHeight(42);

    QPushButton *refreshButton = new QPushButton("Refresh");
    refreshButton->setFixedSize(110,42);

    searchLayout->addWidget(searchBox);
    searchLayout->addWidget(refreshButton);

    mainLayout->addLayout(searchLayout);
    //--------------------------------------------------
    // Student Table
    //--------------------------------------------------

    QTableWidget *table = new QTableWidget();

    table->setColumnCount(6);

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

    mainLayout->addWidget(table);
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
        editItem->setForeground(QBrush(QColor("#3B82F6")));

        QTableWidgetItem *deleteItem = new QTableWidgetItem("Delete");
        deleteItem->setTextAlignment(Qt::AlignCenter);
        deleteItem->setForeground(QBrush(QColor("#EF4444")));

        table->setItem(row,3,editItem);
        table->setItem(row,4,deleteItem);
    }
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    QLabel *totalStudents = new QLabel("Total Registered Students : 5");
    totalStudents->setStyleSheet(
        "color:#94A3B8;"
        "font-size:14px;"
        );

    mainLayout->addWidget(totalStudents);

    setLayout(mainLayout);
}