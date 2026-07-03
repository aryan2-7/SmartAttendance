
#include "db.h"
#include <iostream>
#include <ctime>   


Database::Database(const std::string& dbPath) : db(nullptr) {
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << "\n";
        db = nullptr;
    }else{
        execute("PRAGMA foreign_keys = ON;");
    }
}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
    }
}
bool Database::execute(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << "\n";
        sqlite3_free(errMsg);
        return false;
    }
    return true;
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
        "  time       TEXT    NOT NULL,"
        "  FOREIGN KEY (rollNumber) REFERENCES students(rollNumber)"
        ");";
    
     std::string defaultUser =
        "INSERT OR IGNORE INTO users (username, password) "
        "VALUES ('admin', 'admin123');";

     return execute(users) && execute(students) &&  execute(attendance) && execute(defaultUser);

}
bool Database::addStudent(const std::string& name, int rollNumber) {
    std::string sql =
        "INSERT OR IGNORE INTO students (name, rollNumber) VALUES ('" +
        name + "', " + std::to_string(rollNumber) + ");";
    return execute(sql);
}


bool Database::studentExists(int rollNumber) {
    std::string sql =
        "SELECT COUNT(*) FROM students WHERE rollNumber = " + std::to_string(rollNumber) + ";";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_step(stmt);
    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return count > 0;
}

// ---------- Attendance ----------


bool Database::markAttendance(int rollNumber, const std::string& name) {
    // Get today's date and current time
    time_t now = time(nullptr);
    char dateBuf[11], timeBuf[9];
    strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", localtime(&now));
    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", localtime(&now));


    // Don't allow duplicate marking on the same day
    std::string check =
        "SELECT COUNT(*) FROM attendance WHERE rollNumber = " +
        std::to_string(rollNumber) + " AND date = '" + dateBuf + "';";


    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, check.c_str(), -1, &stmt, nullptr);
    sqlite3_step(stmt);
    int already = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    if (already > 0) return false;  
    std::string sql =
        "INSERT INTO attendance (rollNumber, name, date, time) VALUES (" +
        std::to_string(rollNumber) + ", '" + name + "', '" +
        dateBuf + "', '" + timeBuf + "');";
    return execute(sql);
}


std::vector<AttendanceRecord> Database::getAllRecords() {
    std::vector<AttendanceRecord> records;
    std::string sql =
        "SELECT id, name, rollNumber, date, time FROM attendance "
        "ORDER BY date DESC, time DESC;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AttendanceRecord r;
        r.id          = sqlite3_column_int(stmt, 0);
        r.studentName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        r.rollNumber  = sqlite3_column_int(stmt, 2);
        r.date        = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        r.time        = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        records.push_back(r);
    }
    sqlite3_finalize(stmt);
    return records;
}

bool Database::checkLogin(const std::string& username,
                          const std::string& password) {
    std::string sql =
        "SELECT COUNT(*) FROM users WHERE username = '" + username +
        "' AND password = '" + password + "';";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_step(stmt);
    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return count > 0;
}


