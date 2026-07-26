#pragma once
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <string>

inline QString appDataDir() {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir;
}

inline QString appTrainedModelsDir() {
    QString dir = appDataDir() + "/trained_models";
    QDir().mkpath(dir);
    return dir;
}

inline QString bundledResourcesDir() {
#ifdef INSTALLED_BUILD
#ifdef __APPLE__
    return QCoreApplication::applicationDirPath() + "/../Resources";
#elif defined(_WIN32)
    return QCoreApplication::applicationDirPath() + "/resources";
#else
#error "INSTALLED_BUILD not supported on this platform"
#endif
#else
    return QString(PROJECT_SOURCE_DIR) + "/resources";
#endif
}
