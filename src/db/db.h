#pragma once
#include <string>
#include <vector>
#include "sqlite3.h"

struct StudentRecordDB {
    int id;
    std::string name;
    int rollNumber;
    std::string modelPath;
};

struct AttendanceRecord {
    int id;
    std::string studentName;
    int rollNumber;
    std::string date;
    std::string time;
};

class Database {
public:
    explicit Database(const std::string& dbPath);
    ~Database();
    bool initializeTables();

    bool addStudent(const std::string& name, int rollNumber, const std::string& modelPath = "");
    std::vector<StudentRecordDB> getAllStudents();
    bool deleteStudent(int rollNumber);
    bool updateStudent(int oldRollNumber, const std::string& newName, int newRollNumber, const std::string& newModelPath = "");
    bool studentExists(int rollNumber);

    bool markAttendance(int rollNumber, const std::string& name);
    std::vector<AttendanceRecord> getAllRecords();
    bool checkLogin(const std::string& username, const std::string& password);

private:
    sqlite3* db;
    bool execute(const std::string& sql);
    static std::string escapeSql(const std::string& value);
    bool migrateAttendanceTable();
};
