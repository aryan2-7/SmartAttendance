#include "AdministratorWindow.h"
#include "../auth/FontManager.h"
#include "../theme/Theme.h"
#include "../db/db.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include "../auth/WelcomeWindow.h"
#include "AdminDashboard.h"

AdministratorWindow::AdministratorWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void AdministratorWindow::setupUI()
{
    setWindowTitle("Administrator Login");
    resize(1400, 850);
    setMinimumSize(1300, 800);
    setObjectName("AdministratorWindow");

    setStyleSheet(QString(R"(
QWidget#AdministratorWindow{
    background:%1;
    color:%2;
}
)")
                      .arg(Theme::Card)
                      .arg(Theme::Primary));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30,30,30,30);

    // Header
    QHBoxLayout *header = new QHBoxLayout();

    backButton = new QPushButton("← Back");
    connect(backButton, &QPushButton::clicked, this, [this]()
            {
                auto *window = new WelcomeWindow();
                window->show();
                this->close();
            });
    backButton->setFixedSize(110,40);

    backButton->setStyleSheet(QString(R"(
QPushButton{
    background:%1;
    border:1px solid %2;
    border-radius:10px;
    color:%3;
}
QPushButton:hover{
    background:%4;
}
)")
                                   .arg(Theme::Surface)
                                   .arg(Theme::Border)
                                   .arg(Theme::Primary)
                                   .arg(Theme::Hover));

    QLabel *systemTitle = new QLabel("Smart Attendance");
    systemTitle->setFont(FontManager::headingFont(22));
    systemTitle->setStyleSheet(QString("color:%1;").arg(Theme::Gold));

    header->addWidget(backButton);
    header->addStretch();
    header->addWidget(systemTitle);
    header->addStretch();

    mainLayout->addLayout(header);
    mainLayout->addStretch();

    // Login Card
    QFrame *card = new QFrame();
    card->setFixedSize(420,430);

    card->setStyleSheet(QString(R"(
QFrame{
    background:%1;
    border:1px solid %2;
    border-radius:18px;
}
)")
                             .arg(Theme::Surface)
                             .arg(Theme::Border));

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
    title->setStyleSheet(QString("color:%1;").arg(Theme::Primary));
    cardLayout->addWidget(title);

    QLabel *subtitle = new QLabel(
        "Login to manage the Smart Attendance System.");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet(QString("color:%1;").arg(Theme::Secondary));
    cardLayout->addWidget(subtitle);

    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Username");

    passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText("Password");
    passwordEdit->setEchoMode(QLineEdit::Password);

    QString editStyle = QString(R"(
QLineEdit{
    background:%1;
    border:1px solid %2;
    border-radius:10px;
    padding:12px;
    color:%3;
}
QLineEdit:focus{
    border:1px solid %4;
}
)")
                             .arg(Theme::Input)
                             .arg(Theme::Border)
                             .arg(Theme::Primary)
                             .arg(Theme::Gold);

    usernameEdit->setStyleSheet(editStyle);
    passwordEdit->setStyleSheet(editStyle);

    cardLayout->addWidget(usernameEdit);
    cardLayout->addWidget(passwordEdit);

    auto doLogin = [this]() {
        Database db(std::string(PROJECT_SOURCE_DIR) + "/smart_attendance.db");
        db.initializeTables();
        if (db.checkLogin(usernameEdit->text().toStdString(),
                          passwordEdit->text().toStdString())) {
            auto *window = new AdminDashboard();
            window->show();
            this->close();
        } else {
            QMessageBox::critical(this, "Login Failed",
                                  "Invalid username or password.");
        }
    };

    loginButton = new QPushButton("Login");
    connect(loginButton, &QPushButton::clicked, this, doLogin);
    // Also submit on Enter key from password field
    connect(passwordEdit, &QLineEdit::returnPressed, this, doLogin);
    connect(usernameEdit, &QLineEdit::returnPressed, this, [this]() {
        passwordEdit->setFocus();
    });
    loginButton->setMinimumHeight(45);

    loginButton->setStyleSheet(QString(R"(
QPushButton{
    background:%1;
    color:%2;
    border:none;
    border-radius:10px;
    font-size:15px;
    font-weight:bold;
}
QPushButton:hover{
    background:%3;
}
)")
                                    .arg(Theme::Gold)
                                    .arg(Theme::Card)
                                    .arg(Theme::Warning));

    cardLayout->addWidget(loginButton);

    mainLayout->addWidget(card,0,Qt::AlignCenter);

    mainLayout->addStretch();

    setLayout(mainLayout);

    // Auto-focus username field
    usernameEdit->setFocus();
}
