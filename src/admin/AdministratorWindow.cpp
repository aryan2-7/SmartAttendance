#include "AdministratorWindow.h"
#include "../auth/FontManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QCheckBox>

AdministratorWindow::AdministratorWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void AdministratorWindow::setupUI()
{
    setWindowTitle("Administrator Login");
    resize(900, 650);

    setStyleSheet(R"(
QWidget{
    background:#0F172A;
    color:white;
}
)");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30,30,30,30);

    // ================= Header =================

    QHBoxLayout *header = new QHBoxLayout();

    backButton = new QPushButton("← Back");
    backButton->setFixedSize(110,40);

    backButton->setStyleSheet(R"(
QPushButton{
    background:#1E293B;
    border:1px solid #334155;
    border-radius:10px;
    color:white;
}
QPushButton:hover{
    background:#334155;
}
)");

    QLabel *systemTitle = new QLabel("Smart Attendance");
    systemTitle->setFont(FontManager::headingFont(22));

    header->addWidget(backButton);
    header->addStretch();
    header->addWidget(systemTitle);
    header->addStretch();

    mainLayout->addLayout(header);
    mainLayout->addStretch();

    // ================= Login Card =================

    QFrame *card = new QFrame();
    card->setFixedSize(420,430);

    card->setStyleSheet(R"(
QFrame{
    background:#1E293B;
    border:1px solid #334155;
    border-radius:18px;
}
)");

    auto shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(30);
    shadow->setOffset(0,5);
    shadow->setColor(QColor(0,0,0,150));
    card->setGraphicsEffect(shadow);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(35,35,35,35);
    cardLayout->setSpacing(18);

    QLabel *icon = new QLabel("🔒");
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("font-size:48px;");
    cardLayout->addWidget(icon);

    QLabel *title = new QLabel("Administrator Login");
    title->setAlignment(Qt::AlignCenter);
    title->setFont(FontManager::headingFont(20));
    cardLayout->addWidget(title);

    QLabel *subtitle = new QLabel(
        "Login to manage the Smart Attendance System."
        );
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("color:#94A3B8;");
    cardLayout->addWidget(subtitle);

    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Username");

    passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText("Password");
    passwordEdit->setEchoMode(QLineEdit::Password);

    QString editStyle = R"(
QLineEdit{
    background:#0F172A;
    border:1px solid #475569;
    border-radius:10px;
    padding:12px;
    color:white;
}
QLineEdit:focus{
    border:1px solid #3B82F6;
}
)";

    usernameEdit->setStyleSheet(editStyle);
    passwordEdit->setStyleSheet(editStyle);

    cardLayout->addWidget(usernameEdit);
    cardLayout->addWidget(passwordEdit);


    loginButton = new QPushButton("Login");
    loginButton->setMinimumHeight(45);

    loginButton->setStyleSheet(R"(
QPushButton{
    background:#2563EB;
    color:white;
    border:none;
    border-radius:10px;
    font-size:15px;
    font-weight:bold;
}
QPushButton:hover{
    background:#1D4ED8;
}
)");

    cardLayout->addWidget(loginButton);

    mainLayout->addWidget(card,0,Qt::AlignCenter);

    mainLayout->addStretch();

    setLayout(mainLayout);
}