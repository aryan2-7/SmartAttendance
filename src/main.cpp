#include <QApplication>
#include "FaceRegistration.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    FaceRegistration window;
    window.setWindowTitle("Face Registration");
    window.resize(800, 650);
    window.show();

    return app.exec();
}