#pragma once
#include <string>
#include <vector>
#include "sqlite3.h"

struct StudentRecord {
    int studentId;
    std::string studentName;
    std::string studentModelPath;
    int studentRollNumber;
};

struct EnrollmentRecord {
    int enrollmentId;
    int enrollmentStudentId;
    int enrollmentSubjectId;
};


class StudentDAO {
public:
    explicit StudentDAO(sqlite3* db);

    bool addStudent(const std::string& studentName, int studentRollNumber, const std::string& studentModelPath = "");
    std::vector<StudentRecord> getAllStudents();
    bool deleteStudent(int studentRollNumber);
    bool updateStudent(int oldRollNumber, const std::string& newStudentName, int newRollNumber, const std::string& newModelPath = "");
    bool studentExists(int studentRollNumber);
    
    bool enrollStudent(int enrollmentStudentId, int enrollmentSubjectId);
    std::vector<EnrollmentRecord> getEnrollmentsForStudent(int studentId);

private:
    sqlite3* db;
};
