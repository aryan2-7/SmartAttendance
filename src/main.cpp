#include <QApplication>
#include <QtGlobal>
#include <QByteArray>
#include <opencv2/core/utils/logger.hpp>
#include "auth/FontManager.h"
#include "auth/WelcomeWindow.h"

int main(int argc, char *argv[]) {
#ifdef _WIN32
    // FIX for: "Could not find the Qt platform plugin 'windows'"
    qputenv("QT_PLUGIN_PATH", QByteArray("C:\\vcpkg\\installed\\x64-windows\\debug\\Qt6\\plugins"));
#endif

    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);

    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("KathmanduUniversity");
    QCoreApplication::setApplicationName("SmartAttendance");

    a.setFont(FontManager::appFont(12));

    WelcomeWindow w;
    w.showMaximized();

    return a.exec();
}