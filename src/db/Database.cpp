#include "Database.h"
#include <iostream>
#include <sqlite3.h>
#include <string>
#include "UserDAO.h"


//Constructor to initialize the database connection
Database::Database(const std::string& dbPath) : db(nullptr) {
    int rc=sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << "\n";
        sqlite3_close(db);
        db = nullptr;
    } else {
        // Enforce foreign key constraints for table relationships
        execute("PRAGMA foreign_keys = ON;");
    }
}

//Destructor to close the database connection
Database::~Database() {
    if (db) {
        sqlite3_close(db);
    }
}

Database::Database(Database&& other) noexcept : db(other.db) {
    other.db = nullptr;
}

Database& Database::operator=(Database&& other) noexcept {
    if (this != &other) {
        if (db) {
            sqlite3_close(db);
        }
        db = other.db;
        other.db = nullptr;
    }
    return *this;
}

// Exposes the raw connection so DAOs can be constructed with it
sqlite3* Database::getConnection() const {
    return db;
}

// A helper function to run raw SQL commands 
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

//Initialize the database tables if they don't exist
bool Database::initializeTables() {

    // Create the users table for login
    const std::string usersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            userId INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password TEXT NOT NULL,
            salt TEXT NOT NULL
        );
    )";

    //Create students table
    const std::string studentsTable = R"(
        CREATE TABLE IF NOT EXISTS students (
            studentId INTEGER PRIMARY KEY AUTOINCREMENT,
            studentName TEXT NOT NULL,
            studentRollNumber INTEGER UNIQUE NOT NULL,
            modelPath TEXT NOT NULL
        );
    )";

    // Create the subjects table
    const std::string subjectsTable = R"(
        CREATE TABLE IF NOT EXISTS subjects (
            subjectId INTEGER PRIMARY KEY AUTOINCREMENT,
            subjectCode TEXT UNIQUE NOT NULL,
            subjectName TEXT NOT NULL,
            semester INTEGER NOT NULL,
            department TEXT NOT NULL,
            subjectMinAttendance INTEGER DEFAULT 80
        );
    )";

    // Enrollments table
    const std::string enrollmentsTable = R"(
        CREATE TABLE IF NOT EXISTS enrollments (
            enrollmentId INTEGER PRIMARY KEY AUTOINCREMENT,
            enrollmentStudentId INTEGER NOT NULL REFERENCES students(studentId) ON DELETE CASCADE,
            enrollmentSubjectId INTEGER NOT NULL REFERENCES subjects(subjectId) ON DELETE CASCADE,
            UNIQUE(enrollmentStudentId, enrollmentSubjectId)
        );
    )";

    // Create the class sessions table
    const std::string classSessionsTable = R"(
        CREATE TABLE IF NOT EXISTS class_sessions (
            sessionId INTEGER PRIMARY KEY AUTOINCREMENT,
            sessionSubjectId INTEGER NOT NULL REFERENCES subjects(subjectId) ON DELETE CASCADE,
            sessionDate TEXT NOT NULL,
            sessionStartTime TEXT NOT NULL,
            sessionEndTime TEXT NOT NULL,
            sessionRoom TEXT,
            sessionTopic TEXT
        );
    )";


    // Create the attendance table
    const std::string  attendanceTable = R"(
        CREATE TABLE IF NOT EXISTS attendance (
            attendanceId INTEGER PRIMARY KEY AUTOINCREMENT,
            attendanceStudentId INTEGER NOT NULL REFERENCES students(studentId) ON DELETE CASCADE,
            attendanceSessionId INTEGER NOT NULL REFERENCES class_sessions(sessionId) ON DELETE CASCADE,
            attendanceDate TEXT NOT NULL,
            attendanceTime TEXT NOT NULL,
            attendanceStatus TEXT NOT NULL DEFAULT 'present',
            UNIQUE(attendanceStudentId, attendanceSessionId)
        );
    )";

    // Execute the table creation commands
if (!execute(usersTable) || 
    !execute(studentsTable) || 
    !execute(subjectsTable) ||       
    !execute(enrollmentsTable) ||    
    !execute(classSessionsTable) ||  
    !execute(attendanceTable) ) {
    return false;
}
UserDAO userDAO(db);
userDAO.createUser("admin", "admin123");

return true;
}