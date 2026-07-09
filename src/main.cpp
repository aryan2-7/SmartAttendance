#include <QApplication>
#include <QtGlobal>
#include <QByteArray>
#include "auth/FontManager.h"
#include "auth/WelcomeWindow.h"

int main(int argc, char *argv[]) {
    // FIX for: "Could not find the Qt platform plugin 'windows'"
    qputenv("QT_PLUGIN_PATH", QByteArray("C:\\vcpkg\\installed\\x64-windows\\debug\\Qt6\\plugins"));

    QApplication a(argc, argv);

    a.setFont(FontManager::appFont(12));

    WelcomeWindow w;
    w.show();

    return a.exec();
}