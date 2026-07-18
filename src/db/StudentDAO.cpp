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


