
#include "db.h"
#include <iostream>
#include <ctime>   // for getting today's date/time

// ---------- Constructor / Destructor ----------

Database::Database(const std::string& dbPath) : db(nullptr) {
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << "\n";
        db = nullptr;
    }
}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
    }
}
bool Database::initializeTables() {
    // users table — for the login window
    std::string users =
        "CREATE TABLE IF NOT EXISTS users ("
        "  id       INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username TEXT    NOT NULL UNIQUE,"
        "  password TEXT    NOT NULL"
        ");";

    // students table — one row per registered person
    std::string students =
        "CREATE TABLE IF NOT EXISTS students ("
        "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name       TEXT    NOT NULL,"
        "  rollNumber INTEGER NOT NULL UNIQUE"
        ");";

    // attendance table — one row every time someone marks attendance
    std::string attendance =
        "CREATE TABLE IF NOT EXISTS attendance ("
        "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  rollNumber INTEGER NOT NULL,"
        "  name       TEXT    NOT NULL,"
        "  date       TEXT    NOT NULL,"
        "  time       TEXT    NOT NULL"
        ");";


}





