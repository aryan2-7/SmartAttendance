#include "MainMenu.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDir>
#include <iostream>
#include "FaceRegistration.h"
#include "AttendanceMarker.h"

MainMenu::MainMenu(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Smart Attendance System - Main Menu");
    resize(500, 350);

    // Styling for a premium dark theme
    setStyleSheet(
        "QWidget { background-color: #121212; color: #E0E0E0; font-family: 'Segoe UI', Arial, sans-serif; }"
        "QLabel { font-size: 24px; font-weight: bold; color: #00E676; margin-bottom: 20px; }"
        "QPushButton { background-color: #1E1E1E; border: 2px solid #333333; border-radius: 8px; padding: 12px; font-size: 16px; font-weight: bold; min-width: 250px; color: #FFFFFF; }"
        "QPushButton:hover { background-color: #2D2D2D; border-color: #00E676; }"
        "QPushButton:pressed { background-color: #00E676; color: #121212; }"
    );

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    QLabel *titleLabel = new QLabel("Smart Attendance System", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    QPushButton *registerBtn = new QPushButton("Register Face (Qt UI)", this);
    QPushButton *markBtn     = new QPushButton("Mark Attendance (OpenCV)", this);
    QPushButton *exitBtn     = new QPushButton("Exit", this);

    layout->addWidget(registerBtn);
    layout->addSpacing(10);
    layout->addWidget(markBtn);
    layout->addSpacing(20);
    layout->addWidget(exitBtn);

    connect(registerBtn, &QPushButton::clicked, this, &MainMenu::onRegisterClicked);
    connect(markBtn,     &QPushButton::clicked, this, &MainMenu::onMarkAttendanceClicked);
    connect(exitBtn,     &QPushButton::clicked, this, &MainMenu::close);
}

void MainMenu::onRegisterClicked() {
    FaceRegistration *regWindow = new FaceRegistration();
    regWindow->setWindowTitle("Face Registration");
    regWindow->resize(800, 650);
    regWindow->setAttribute(Qt::WA_DeleteOnClose);
    regWindow->show();
}

void MainMenu::onMarkAttendanceClicked() {
    // ── Guard: make sure at least one .yml model exists ──────────────────────
    // AttendanceMarker will auto-discover ALL of them – no manual selection needed.
    QDir modelDir("resources/trained_models/");
    if (!modelDir.exists()) modelDir.mkpath(".");

    QStringList modelFiles = modelDir.entryList(QStringList() << "*.yml", QDir::Files);
    if (modelFiles.isEmpty()) {
        QMessageBox::warning(this, "No Models Found",
            "No trained face models (.yml) found in resources/trained_models/.\n\n"
            "Please register at least one face first using 'Register Face'.");
        return;
    }

    // ── Paths ─────────────────────────────────────────────────────────────────
    const std::string cascadePath    = "resources/haarcascades/haarcascade_frontalface_default.xml";
    const std::string eyeCascadePath = "resources/haarcascades/haarcascade_eye.xml";
    const std::string modelsDir      = "resources/trained_models/";

    std::cout << "Starting Attendance Scanner – loading "
              << modelFiles.size() << " model(s) automatically...\n";

    // ── Hide Qt window while OpenCV scanner runs ──────────────────────────────
    this->hide();

    AttendanceMarker marker;
    if (marker.initialize(cascadePath, eyeCascadePath, modelsDir)) {
        marker.run();   // blocks until user presses 'q' or closes the window
    } else {
        QMessageBox::critical(this, "Initialization Failed",
            "Failed to initialize the attendance marker.\n"
            "Make sure the camera is connected and cascade files are in the resources folder.");
    }

    // ── Restore Qt window when scanner exits ──────────────────────────────────
    this->show();
}
