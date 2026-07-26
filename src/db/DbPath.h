#pragma once
#include <string>

// Centralizes the on-disk location of the SQLite database file.
// Previously this literal ("<source dir>/smart_attendance.db") was
// duplicated across ~10 windows; now every window opens the same
// Database via appDbPath() so there is exactly one place to change it.
inline std::string appDbPath() {
    return std::string(PROJECT_SOURCE_DIR) + "/smart_attendance.db";
}
