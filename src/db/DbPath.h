#pragma once
#include "AppPaths.h"
#include <string>

inline std::string appDbPath() {
    return (appDataDir() + "/smart_attendance.db").toStdString();
}
