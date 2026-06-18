#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "db/Database.h"

class LoginWindow : public QDialog {
    Q_OBJECT  // Required for signals/slots to work — don't remove this line

public:
    // Takes a pointer to the shared database
    explicit LoginWindow(Database* db, QWidget* parent = nullptr);

private slots:
    // This function runs when the Login button is clicked
    void onLoginClicked();

private:
    // UI elements
    QLineEdit*   usernameField;
    QLineEdit*   passwordField;
    QPushButton* loginButton;
    QLabel*      errorLabel;   // shows "Invalid credentials" if login fails

    // Pointer to the database (passed in from main.cpp)
    Database* database;

    // Helper to lay out all the widgets nicely
    void setupUI();
};                



