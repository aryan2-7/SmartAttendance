
#include "db.h"
#include <iostream>
#include <ctime>

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
    const char *sql = "INSERT INTO students (name, rollNumber, modelPath) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, rollNumber);
    sqlite3_bind_text(stmt, 3, modelPath.c_str(), -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::studentExists(int rollNumber) {
    const char *sql = "SELECT COUNT(*) FROM students WHERE rollNumber = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, rollNumber);
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) count = sqlite3_column_int(stmt, 0);
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
    const char *sql = "DELETE FROM students WHERE rollNumber = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, rollNumber);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool Database::updateStudent(int oldRollNumber, const std::string& newName, int newRollNumber, const std::string& newModelPath) {
    if (!execute("BEGIN;")) return false;

    sqlite3_stmt *stmt;
    if (newModelPath.empty()) {
        const char *sql = "UPDATE students SET name = ?, rollNumber = ? WHERE rollNumber = ?;";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            execute("ROLLBACK;");
            return false;
        }
        sqlite3_bind_text(stmt, 1, newName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, newRollNumber);
        sqlite3_bind_int(stmt, 3, oldRollNumber);
    } else {
        const char *sql = "UPDATE students SET name = ?, rollNumber = ?, modelPath = ? WHERE rollNumber = ?;";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            execute("ROLLBACK;");
            return false;
        }
        sqlite3_bind_text(stmt, 1, newName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, newRollNumber);
        sqlite3_bind_text(stmt, 3, newModelPath.empty() ? "" : newModelPath.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, oldRollNumber);
    }

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    if (!ok) {
        execute("ROLLBACK;");
        return false;
    }

    const char *attendanceSql = "UPDATE attendance SET name = ? WHERE rollNumber = ?;";
    if (sqlite3_prepare_v2(db, attendanceSql, -1, &stmt, nullptr) != SQLITE_OK) {
        execute("ROLLBACK;");
        return false;
    }
    sqlite3_bind_text(stmt, 1, newName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, newRollNumber);
    ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    if (!ok) {
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

    const char *checkSql = "SELECT COUNT(*) FROM attendance WHERE rollNumber = ? AND date = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, checkSql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, rollNumber);
    sqlite3_bind_text(stmt, 2, dateBuf, -1, SQLITE_TRANSIENT);
    int already = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) already = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    if (already > 0) return false;

    const char *insertSql = "INSERT INTO attendance (rollNumber, name, date, time) VALUES (?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, rollNumber);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, dateBuf, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, timeBuf, -1, SQLITE_TRANSIENT);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
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
    const char *sql = "SELECT COUNT(*) FROM users WHERE username = ? AND password = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return count > 0;
}
