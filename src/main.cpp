#include <QApplication>

#include "auth/FontManager.h"
#include "auth/WelcomeWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setFont(FontManager::appFont(12));

    WelcomeWindow w;
    w.show();

    return a.exec();
}