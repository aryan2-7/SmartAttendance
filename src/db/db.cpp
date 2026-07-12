
#include "db.h"
#include <iostream>
#include <ctime>
#include <sstream>

Database::Database(const std::string& dbPath) : db(nullptr) {
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << "\n";
        db = nullptr;
    } else {
        execute("PRAGMA foreign_keys = ON;");
    }
}

Database::~Database() {
    if (db) sqlite3_close(db);
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

std::string Database::escapeSql(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (char c : value) {
        if (c == '\'') escaped += "''";
        else escaped += c;
    }
    return escaped;
}

bool Database::migrateAttendanceTable() {
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db,
            "SELECT on_update, on_delete FROM pragma_foreign_key_list('attendance') "
            "WHERE \"table\" = 'students' AND \"from\" = 'rollNumber' LIMIT 1;",
            -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    bool needsMigration = true;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* onUpdate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* onDelete = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (onUpdate && onDelete &&
            std::string(onUpdate) == "CASCADE" &&
            std::string(onDelete) == "CASCADE") {
            needsMigration = false;
        }
    }
    sqlite3_finalize(stmt);

    if (!needsMigration) return true;

    const char* migration =
        "BEGIN;"
        "CREATE TABLE attendance_new ("
        "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  rollNumber INTEGER NOT NULL,"
        "  name       TEXT    NOT NULL,"
        "  date       TEXT    NOT NULL,"
        "  time       TEXT    NOT NULL,"
        "  FOREIGN KEY (rollNumber) REFERENCES students(rollNumber) "
        "    ON UPDATE CASCADE ON DELETE CASCADE"
        ");"
        "INSERT INTO attendance_new (id, rollNumber, name, date, time) "
        "SELECT id, rollNumber, name, date, time FROM attendance;"
        "DROP TABLE attendance;"
        "ALTER TABLE attendance_new RENAME TO attendance;"
        "COMMIT;";

    return execute(migration);
}

bool Database::initializeTables() {
    std::string users =
        "CREATE TABLE IF NOT EXISTS users ("
        "  id       INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username TEXT    NOT NULL UNIQUE,"
        "  password TEXT    NOT NULL"
        ");";
    std::string students =
        "CREATE TABLE IF NOT EXISTS students ("
        "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name       TEXT    NOT NULL,"
        "  modelPath  TEXT    NOT NULL,"
        "  rollNumber INTEGER NOT NULL UNIQUE"
        ");";
    std::string attendance =
        "CREATE TABLE IF NOT EXISTS attendance ("
        "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  rollNumber INTEGER NOT NULL,"
        "  name       TEXT    NOT NULL,"
        "  date       TEXT    NOT NULL,"
        "  time       TEXT    NOT NULL,"
        "  FOREIGN KEY (rollNumber) REFERENCES students(rollNumber) "
        "    ON UPDATE CASCADE ON DELETE CASCADE"
        ");";
    std::string defaultUser =
        "INSERT OR IGNORE INTO users (username, password) "
        "VALUES ('admin', 'admin123');";
    if (!execute(users) || !execute(students) || !execute(attendance) || !execute(defaultUser)) {
        return false;
    }
    return migrateAttendanceTable();
}

bool Database::addStudent(const std::string& name, int rollNumber, const std::string& modelPath) {
    if (studentExists(rollNumber)) return false;
    std::string sql =
        "INSERT INTO students (name, rollNumber, modelPath) VALUES ('" +
        escapeSql(name) + "', " + std::to_string(rollNumber) + ", '" +
        escapeSql(modelPath) + "');";
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

std::vector<StudentRecordDB> Database::getAllStudents() {
    std::vector<StudentRecordDB> students;
    std::string sql = "SELECT id, name, rollNumber, modelPath FROM students;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            StudentRecordDB s;
            s.id = sqlite3_column_int(stmt, 0);
            s.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            s.rollNumber = sqlite3_column_int(stmt, 2);
            const char* path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            if (path) s.modelPath = path;
            students.push_back(s);
        }
    }
    sqlite3_finalize(stmt);
    return students;
}

bool Database::deleteStudent(int rollNumber) {
    std::string sql = "DELETE FROM students WHERE rollNumber = " + std::to_string(rollNumber) + ";";
    return execute(sql);
}

bool Database::updateStudent(int oldRollNumber, const std::string& newName, int newRollNumber, const std::string& newModelPath) {
    if (!execute("BEGIN;")) return false;

    std::ostringstream sql;
    sql << "UPDATE students SET name = '" << escapeSql(newName)
        << "', rollNumber = " << newRollNumber;
    if (!newModelPath.empty()) {
        sql << ", modelPath = '" << escapeSql(newModelPath) << "'";
    }
    sql << " WHERE rollNumber = " << oldRollNumber << ";";

    if (!execute(sql.str())) {
        execute("ROLLBACK;");
        return false;
    }

    std::string attendanceSql =
        "UPDATE attendance SET name = '" + escapeSql(newName) +
        "' WHERE rollNumber = " + std::to_string(newRollNumber) + ";";
    if (!execute(attendanceSql)) {
        execute("ROLLBACK;");
        return false;
    }

    execute("COMMIT;");
    return true;
}

bool Database::markAttendance(int rollNumber, const std::string& name) {
    if (!studentExists(rollNumber)) return false;

    time_t now = time(nullptr);
    char dateBuf[11], timeBuf[9];
    strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", localtime(&now));
    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", localtime(&now));

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
        std::to_string(rollNumber) + ", '" + escapeSql(name) + "', '" +
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

bool Database::checkLogin(const std::string& username, const std::string& password) {
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
