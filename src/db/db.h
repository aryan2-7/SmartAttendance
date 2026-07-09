#pragma once
#include <string>
#include <vector>
#include "sqlite3.h"

// One row of attendance data 
struct AttendanceRecord {
    int id;
    std::string studentName;
    int rollNumber;
    std::string date;      
    std::string time;       
};

class Database {
public:
    // Opens (or creates) the database file at the given path
    explicit Database(const std::string& dbPath);

    // Close the DB 
    ~Database();

    // Call this once at start to create tables if they don't exist 
    bool initializeTables();

    // Student
    bool addStudent(const std::string& name, int rollNumber);
    bool studentExists(int rollNumber);

    // --- Attendance ---
    // Returns false if this student already marked attendance today
    bool markAttendance(int rollNumber, const std::string& name);

    // Returns all attendance records (for the RecordsViewer)
    std::vector<AttendanceRecord> getAllRecords();

    // --- Login ---
    // Returns true if username + password match a record in the users table
    bool checkLogin(const std::string& username, const std::string& password);

private:
    sqlite3* db;  // raw SQLite connection handle

    // Internal helper — runs a SQL string, returns true on success
    bool execute(const std::string& sql);
};