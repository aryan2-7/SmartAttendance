#include "AttendanceRecordsWindow.h"
#include "../auth/FontManager.h"
#include "AdminDashboard.h"


#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QDateEdit>
#include <QDate>
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

    setStyleSheet(R"(
QWidget{
    background:#0F172A;
    color:white;
}
QTableWidget{
    background:#111827;
    border:1px solid #334155;
    border-radius:14px;
    gridline-color:#334155;
    color:white;
    font-size:14px;
    selection-background-color:#2563EB;
}

QHeaderView::section{
    background:#1E293B;
    color:white;
    border:none;
    padding:10px;
    font-weight:bold;
}

QTableWidget::item{
    background:#111827;
    color:white;
}

QPushButton{
    background:#1E293B;
    color:white;
    border:1px solid #334155;
    border-radius:10px;
    padding:10px 18px;
}
QLineEdit,
QDateEdit{
    background:#111827;
    color:white;
    border:1px solid #334155;
    border-radius:10px;
    padding:10px;
    font-size:14px;
}

QLineEdit:focus,
QDateEdit:focus{
    border:1px solid #2563EB;
}

QPushButton:hover{
    background:#334155;
}

QLabel{
    border:none;
    background:transparent;
}
)");

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

    header->addWidget(backButton);
    header->addSpacing(20);
    header->addWidget(title);
    header->addStretch();

    mainLayout->addLayout(header);
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

    mainLayout->addLayout(filterLayout);
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

    mainLayout->addWidget(table);
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
            status->setForeground(QBrush(QColor("#22C55E")));
        else if(status->text()=="Late")
            status->setForeground(QBrush(QColor("#FACC15")));
        else
            status->setForeground(QBrush(QColor("#EF4444")));

        status->setTextAlignment(Qt::AlignCenter);
    }
    setLayout(mainLayout);
}