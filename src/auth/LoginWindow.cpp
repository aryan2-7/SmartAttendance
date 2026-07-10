#include "LoginWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QLabel>

LoginWindow::LoginWindow(Database* db, QWidget* parent)
    : QDialog(parent), database(db)
{
    setWindowTitle("Smart Attendance — Login");
    setFixedSize(350, 200);
    setupUI();
}

void LoginWindow::setupUI() {
    // --- Create widgets ---
    QLabel* usernameLabel = new QLabel("Username:");
    QLabel* passwordLabel = new QLabel("Password:");

    usernameField = new QLineEdit();
    usernameField->setPlaceholderText("Enter username");

    passwordField = new QLineEdit();
    passwordField->setPlaceholderText("Enter password");
    passwordField->setEchoMode(QLineEdit::Password);  // hides text as ****

    loginButton = new QPushButton("Login");

    errorLabel = new QLabel("");
    errorLabel->setStyleSheet("color: red;");  // red text for errors

    // --- Layout ---
    // QVBoxLayout stacks things vertically (top to bottom)
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(30, 20, 30, 20);

    mainLayout->addWidget(usernameLabel);
    mainLayout->addWidget(usernameField);
    mainLayout->addWidget(passwordLabel);
    mainLayout->addWidget(passwordField);
    mainLayout->addWidget(loginButton);
    mainLayout->addWidget(errorLabel);

    // --- Connect button click to our slot ---
    // Syntax: connect(who_emits, signal, who_receives, slot_function)
    connect(loginButton, &QPushButton::clicked,
        this, &LoginWindow::onLoginClicked);

    // Also let the user press Enter in the password field to login
    connect(passwordField, &QLineEdit::returnPressed,
        this, &LoginWindow::onLoginClicked);
}

void LoginWindow::onLoginClicked() {
    QString username = usernameField->text().trimmed();
    QString password = passwordField->text();


    // Basic check — don't even query the DB if fields are empty
    if (username.isEmpty() || password.isEmpty()) {
        errorLabel->setText("Please fill in both fields.");
        return;
    }

    // Ask the database
    bool valid = database->checkLogin(
        username.toStdString(),
        password.toStdString()
    );


    if (valid) {
        // QDialog::accept() closes this dialog and returns QDialog::Accepted
        // main.cpp checks this return value to know login succeeded
        accept();
    }

    else {
        errorLabel->setText("Invalid username or password.");
        passwordField->clear();
        passwordField->setFocus();
    }
}