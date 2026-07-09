#include "AttendanceRecordsWindow.h"
#include "../auth/FontManager.h"
#include "AdminDashboard.h"
#include "../theme/Theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QDateEdit>
#include <QDate>
#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QAbstractItemView>

AttendanceRecordsWindow::AttendanceRecordsWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void AttendanceRecordsWindow::setupUI()
{
    setWindowTitle("Attendance Records");
    resize(1400,850);
    setObjectName("AttendanceRecordsWindow");

    setStyleSheet(QString(R"(

QWidget#AttendanceRecordsWindow{
    background:%1;
}

QFrame#card{
    background:%3;
    border:1px solid %2;
    border-radius:16px;
}

QTableWidget{
    background:%6;
    border:1px solid %2;
    border-radius:14px;
    gridline-color:%2;
    color:%4;
    font-size:14px;
    selection-background-color:%7;
}

QHeaderView::section{
    background:%3;
    color:%4;
    border:none;
    padding:10px;
    font-weight:bold;
}

QTableWidget::item{
    background:%6;
    color:%4;
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

QLineEdit,
QDateEdit{
    background:%6;
    color:%4;
    border:1px solid %2;
    border-radius:10px;
    padding:10px;
    font-size:14px;
}

QLineEdit:focus,
QDateEdit:focus{
    border:1px solid %7;
}

QLabel{
    border:none;
    background:transparent;
    color:%4;
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

    //---------------------------------------
    // Header
    //---------------------------------------

    QHBoxLayout *header = new QHBoxLayout();

    QPushButton *backButton = new QPushButton("Back");
    connect(backButton, &QPushButton::clicked, this, [this]()
            {
                auto *window = new AdminDashboard();
                window->show();
                this->close();
            });
    backButton->setFixedSize(100,40);

    QLabel *title = new QLabel("Attendance Records");
    title->setFont(FontManager::headingFont(26));
    title->setStyleSheet(QString("color:%1;").arg(Theme::Gold));

    header->addWidget(backButton);
    header->addSpacing(20);
    header->addWidget(title);
    header->addStretch();

    mainLayout->addLayout(header);

    //--------------------------------------------------
    // Card container (search/filter + table live inside this,
    // styled to match the boxed "data" panel used across the app)
    //--------------------------------------------------

    QFrame *card = new QFrame();
    card->setObjectName("card");

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20,20,20,20);
    cardLayout->setSpacing(20);

    //--------------------------------------------------
    // Search & Filter
    //--------------------------------------------------

    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(15);

    QLineEdit *searchBox = new QLineEdit();
    searchBox->setPlaceholderText("Search Student...");
    searchBox->setFixedHeight(42);

    QDateEdit *dateFilter = new QDateEdit();
    dateFilter->setCalendarPopup(true);
    dateFilter->setDate(QDate::currentDate());
    dateFilter->setFixedSize(160,42);

    QPushButton *refreshButton = new QPushButton("Refresh");
    refreshButton->setFixedSize(110,42);

    filterLayout->addWidget(searchBox);
    filterLayout->addWidget(dateFilter);
    filterLayout->addWidget(refreshButton);

    cardLayout->addLayout(filterLayout);
    //--------------------------------------------------
    // Attendance Table
    //--------------------------------------------------

    QTableWidget *table = new QTableWidget();

    table->setColumnCount(6);

    table->setHorizontalHeaderLabels({
        "Student ID",
        "Student Name",
        "Roll No",
        "Date",
        "Time",
        "Attendance"
    });

    table->verticalHeader()->hide();
    table->setAlternatingRowColors(false);
    table->setShowGrid(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setFocusPolicy(Qt::NoFocus);
    table->verticalHeader()->setDefaultSectionSize(45);

    table->horizontalHeader()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(2,QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(3,QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(4,QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(5,QHeaderView::ResizeToContents);

    cardLayout->addWidget(table);

    // Card is fully built now — add it to the page
    mainLayout->addWidget(card);
    //--------------------------------------------------
    // Sample Attendance Records
    //--------------------------------------------------

    table->setRowCount(5);

    table->setItem(0,0,new QTableWidgetItem("1"));
    table->setItem(0,1,new QTableWidgetItem("Arhan Watson"));
    table->setItem(0,2,new QTableWidgetItem("15"));
    table->setItem(0,3,new QTableWidgetItem("08 Jul 2026"));
    table->setItem(0,4,new QTableWidgetItem("08:45 AM"));
    table->setItem(0,5,new QTableWidgetItem("Present"));

    table->setItem(1,0,new QTableWidgetItem("2"));
    table->setItem(1,1,new QTableWidgetItem("Emma Wilson"));
    table->setItem(1,2,new QTableWidgetItem("22"));
    table->setItem(1,3,new QTableWidgetItem("08 Jul 2026"));
    table->setItem(1,4,new QTableWidgetItem("09:03 AM"));
    table->setItem(1,5,new QTableWidgetItem("Late"));

    table->setItem(2,0,new QTableWidgetItem("3"));
    table->setItem(2,1,new QTableWidgetItem("John Smith"));
    table->setItem(2,2,new QTableWidgetItem("08"));
    table->setItem(2,3,new QTableWidgetItem("08 Jul 2026"));
    table->setItem(2,4,new QTableWidgetItem("08:39 AM"));
    table->setItem(2,5,new QTableWidgetItem("Present"));

    table->setItem(3,0,new QTableWidgetItem("4"));
    table->setItem(3,1,new QTableWidgetItem("Sophia Brown"));
    table->setItem(3,2,new QTableWidgetItem("19"));
    table->setItem(3,3,new QTableWidgetItem("08 Jul 2026"));
    table->setItem(3,4,new QTableWidgetItem("08:58 AM"));
    table->setItem(3,5,new QTableWidgetItem("Present"));

    table->setItem(4,0,new QTableWidgetItem("5"));
    table->setItem(4,1,new QTableWidgetItem("David Miller"));
    table->setItem(4,2,new QTableWidgetItem("11"));
    table->setItem(4,3,new QTableWidgetItem("08 Jul 2026"));
    table->setItem(4,4,new QTableWidgetItem("09:12 AM"));
    table->setItem(4,5,new QTableWidgetItem("Absent"));

    for(int row=0; row<table->rowCount(); row++)
    {
        QTableWidgetItem *status = table->item(row,5);

        if(status->text()=="Present")
            status->setForeground(QBrush(QColor(Theme::Success)));
        else if(status->text()=="Late")
            status->setForeground(QBrush(QColor(Theme::Warning)));
        else
            status->setForeground(QBrush(QColor(Theme::Danger)));

        status->setTextAlignment(Qt::AlignCenter);
    }
    setLayout(mainLayout);
}