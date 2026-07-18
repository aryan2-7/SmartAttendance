#include "StudentDAO.h"
#include <iostream>


//Constructor to initialize the StudentDAO with a database connection
StudentDAO::StudentDAO(sqlite3* db) : db(db) {}

//Check if student exists in the database by roll number
bool StudentDAO::studentExists(int studentRollNumber) {
    const char* sql = "SELECT COUNT(*) FROM students WHERE studentRollNumber = ?;";
    sqlite3_stmt* stmt = nullptr;
   
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, studentRollNumber);
   
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return count > 0;
}

// Inserts a new student record into the system
bool StudentDAO::addStudent(const std::string& studentName, int studentRollNumber, const std::string& studentModelPath) {
    if (studentExists(studentRollNumber)) return false;

    const char* sql = "INSERT INTO students (studentName, studentRollNumber, modelPath) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
   
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, studentName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, studentRollNumber);
    sqlite3_bind_text(stmt, 3, studentModelPath.c_str(), -1, SQLITE_TRANSIENT);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

// Deletes a student by their roll number
bool StudentDAO::deleteStudent(int studentRollNumber) {
    const char* sql = "DELETE FROM students WHERE studentRollNumber = ?;";
    sqlite3_stmt* stmt = nullptr;
   
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, studentRollNumber);
   
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

// Updates an existing student's information
bool StudentDAO::updateStudent(int oldRollNumber, const std::string& newStudentName, int newRollNumber, const std::string& newModelPath) {
    sqlite3_stmt* stmt = nullptr;

    if (newModelPath.empty()) {
        const char* sql = "UPDATE students SET studentName = ?, studentRollNumber = ? WHERE studentRollNumber = ?;";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_text(stmt, 1, newStudentName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, newRollNumber);
        sqlite3_bind_int(stmt, 3, oldRollNumber);
    } else {
        const char* sql = "UPDATE students SET studentName = ?, studentRollNumber = ?, modelPath = ? WHERE studentRollNumber = ?;";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_text(stmt, 1, newStudentName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, newRollNumber);
        sqlite3_bind_text(stmt, 3, newModelPath.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, oldRollNumber);
    }
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

std::vector<StudentRecord> StudentDAO::getAllStudents() {
    std::vector<StudentRecord> recordList;
    const char* sql = "SELECT studentId, studentName, studentRollNumber, modelPath FROM students;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            StudentRecord s;
            s.studentId = sqlite3_column_int(stmt, 0);
            const char* nameText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            if (nameText) {
                s.studentName = nameText;
            }
            s.studentRollNumber = sqlite3_column_int(stmt, 2);
            const char* path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            if (path) s.studentModelPath = path;
            recordList.push_back(s);
        }
    }
    sqlite3_finalize(stmt);
    return recordList;
}

// Links a student ID to a subject ID in the enrollments table
bool StudentDAO::enrollStudent(int enrollmentStudentId, int enrollmentSubjectId) {
    const char* sql = "INSERT INTO enrollments (enrollmentStudentId, enrollmentSubjectId) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
   
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
   
    sqlite3_bind_int(stmt, 1, enrollmentStudentId);
    sqlite3_bind_int(stmt, 2, enrollmentSubjectId);
   
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

// Returns every subject a student is enrolled in.
std::vector<EnrollmentRecord> StudentDAO::getEnrollmentsForStudent(int studentId) {
    std::vector<EnrollmentRecord> records;
    const char* sql = "SELECT enrollmentId, enrollmentStudentId, enrollmentSubjectId FROM enrollments WHERE enrollmentStudentId = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return records;
    }
    sqlite3_bind_int(stmt, 1, studentId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        EnrollmentRecord record;
        record.enrollmentId = sqlite3_column_int(stmt, 0);
        record.enrollmentStudentId = sqlite3_column_int(stmt, 1);
        record.enrollmentSubjectId = sqlite3_column_int(stmt, 2);
        records.push_back(record);
 }
    sqlite3_finalize(stmt);
    return records;
}









