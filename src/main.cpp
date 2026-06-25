#include <QApplication>
#include <QtGlobal>
#include <QByteArray>
#include "MainMenu.h"

int main(int argc, char *argv[]) {
    // FIX for: "Could not find the Qt platform plugin 'windows'"
    qputenv("QT_PLUGIN_PATH", QByteArray("C:\\vcpkg\\installed\\x64-windows\\debug\\Qt6\\plugins"));

    QApplication app(argc, argv);

    MainMenu menu;
    menu.show();

    return app.exec();
}