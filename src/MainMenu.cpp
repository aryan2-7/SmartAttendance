#include "MainMenu.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDir>
#include <QInputDialog>
#include <QFileInfo>
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
    QPushButton *markBtn = new QPushButton("Mark Attendance (OpenCV)", this);
    QPushButton *exitBtn = new QPushButton("Exit", this);

    layout->addWidget(registerBtn);
    layout->addSpacing(10);
    layout->addWidget(markBtn);
    layout->addSpacing(20);
    layout->addWidget(exitBtn);

    connect(registerBtn, &QPushButton::clicked, this, &MainMenu::onRegisterClicked);
    connect(markBtn, &QPushButton::clicked, this, &MainMenu::onMarkAttendanceClicked);
    connect(exitBtn, &QPushButton::clicked, this, &MainMenu::close);
}

void MainMenu::onRegisterClicked() {
    FaceRegistration *regWindow = new FaceRegistration();
    regWindow->setWindowTitle("Face Registration");
    regWindow->resize(800, 650);
    regWindow->setAttribute(Qt::WA_DeleteOnClose);
    regWindow->show();
}

void MainMenu::onMarkAttendanceClicked() {
    QDir modelDir("resources/trained_models/");
    if (!modelDir.exists()) {
        modelDir.mkpath(".");
    }

    QStringList filters;
    filters << "*.yml";
    QStringList modelFiles = modelDir.entryList(filters, QDir::Files);

    if (modelFiles.isEmpty()) {
        QMessageBox::warning(this, "No Models Found",
            "No trained face models (.yml) found in resources/trained_models/.\n\n"
            "Please register a face first using 'Register Face'.");
        return;
    }

    QString selectedModel;
    if (modelFiles.size() == 1) {
        selectedModel = modelFiles.first();
    } else {
        bool ok = false;
        QString item = QInputDialog::getItem(this, "Select Face Model",
            "Select the face model to recognize:", modelFiles, 0, false, &ok);
        if (ok && !item.isEmpty()) {
            selectedModel = item;
        } else {
            return; // User cancelled
        }
    }

    // Parse Name and Roll from model filename (e.g. "Shashwot_Karki_36.yml" -> Name: "Shashwot Karki", Roll: "36")
    QString baseName = QFileInfo(selectedModel).baseName();
    int lastUnderscore = baseName.lastIndexOf('_');
    QString rollNo = "Unknown";
    QString name = baseName;
    if (lastUnderscore != -1) {
        rollNo = baseName.mid(lastUnderscore + 1);
        name = baseName.left(lastUnderscore);
        name.replace('_', ' ');
    }

    std::string cascadePath = "resources/haarcascades/haarcascade_frontalface_default.xml";
    std::string eyeCascadePath = "resources/haarcascades/haarcascade_eye.xml";
    std::string modelPath = "resources/trained_models/" + selectedModel.toStdString();

    AttendanceMarker marker;
    
    // Populate label map for predictions
    std::unordered_map<int, std::pair<std::string, std::string>> labelMap;
    labelMap[1] = { name.toStdString(), rollNo.toStdString() };
    marker.setLabelMap(labelMap);

    std::cout << "Starting Attendance Scanner for: " << name.toStdString() << " (Roll No: " << rollNo.toStdString() << ")\n";

    if (marker.initialize(cascadePath, eyeCascadePath, modelPath)) {
        // Hide Qt window only after camera is confirmed open
        this->hide();
        marker.run();
        // Show Qt window again after scanner closes
        this->show();
    } else {
        QMessageBox::critical(this, "Initialization Failed",
            "Failed to initialize the attendance marker. Make sure camera and cascade files are accessible.");
        this->show();
    }
}
