#include "SubjectDAO.h"
#include <iostream>


// Constructor: Accepts the connection pointer from the main Database coordinator
SubjectDAO::SubjectDAO(sqlite3* databaseConnection) : db(databaseConnection) {}


// Inserts subjects into the database
bool SubjectDAO::createSubject(const std::string& subjectCode, const std::string& subjectName, int subjectSemester, const std::string& subjectDepartment, int subjectMinAttendance) {
    const char* sql = "INSERT INTO subjects (subjectCode, subjectName, semester, department, subjectMinAttendance) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
   
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, subjectCode.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, subjectName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, subjectSemester);
    sqlite3_bind_text(stmt, 4, subjectDepartment.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, subjectMinAttendance);
   
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}


// Retrieves all courses
std::vector<SubjectRecord> SubjectDAO::getAllSubjects() {
    std::vector<SubjectRecord> subjectList;
    const char* sql = "SELECT subjectId, subjectCode, subjectName, semester, department, subjectMinAttendance FROM subjects;";
    sqlite3_stmt* stmt = nullptr;
   
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            SubjectRecord record;
            record.subjectId = sqlite3_column_int(stmt, 0);
            record.subjectCode = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            record.subjectName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            record.subjectSemester = sqlite3_column_int(stmt, 3);
            record.subjectDepartment = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            record.subjectMinAttendance = sqlite3_column_int(stmt, 5);
           
            subjectList.push_back(record);
        }
    }
    sqlite3_finalize(stmt);
    return subjectList;
}


// Creates a scheduled class session
bool SubjectDAO::createClassSession(int sessionSubjectId, const std::string& sessionDate, const std::string& sessionStartTime, const std::string& sessionEndTime, const std::string& sessionRoom, const std::string& sessionTopic) {
    const char* sql = "INSERT INTO class_sessions (sessionSubjectId, sessionDate, sessionStartTime, sessionEndTime, sessionRoom, sessionTopic) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
   
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
   
    sqlite3_bind_int(stmt, 1, sessionSubjectId);
    sqlite3_bind_text(stmt, 2, sessionDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, sessionStartTime.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, sessionEndTime.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, sessionRoom.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, sessionTopic.c_str(), -1, SQLITE_TRANSIENT);
    
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

std::vector<ClassSessionRecord> SubjectDAO::getSessionsForSubject(int subjectId) {
    std::vector<ClassSessionRecord> sessionList;
    const char* sql = "SELECT sessionId, sessionSubjectId, sessionDate, sessionStartTime, sessionEndTime, sessionRoom, sessionTopic FROM class_sessions WHERE sessionSubjectId = ? ORDER BY sessionDate, sessionStartTime;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, subjectId);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            ClassSessionRecord record;
            record.sessionId = sqlite3_column_int(stmt, 0);
            record.sessionSubjectId = sqlite3_column_int(stmt, 1);
            record.sessionDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            record.sessionStartTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            record.sessionEndTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            record.sessionRoom = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            record.sessionTopic = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            sessionList.push_back(record);
        }
    }
    sqlite3_finalize(stmt);
    return sessionList;
}

bool SubjectDAO::deleteClassSession(int sessionId) {
    const char* sql = "DELETE FROM class_sessions WHERE sessionId = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, sessionId);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

