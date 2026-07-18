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







