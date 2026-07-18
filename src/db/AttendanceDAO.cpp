#include "AttendanceDAO.h"
#include <iostream>
#include <ctime>

// Constructor
AttendanceDAO::AttendanceDAO(sqlite3* databaseConnection) : db(databaseConnection) {}


//Confirms a student is enrolled in a subject.
bool AttendanceDAO::isEnrolled(int studentId, int subjectId) {
    const char* sql = "SELECT COUNT(*) FROM enrollments WHERE enrollmentStudentId = ? AND enrollmentSubjectId = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, studentId);
    sqlite3_bind_int(stmt, 2, subjectId);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count > 0;
}

// Finds the class session scheduled for right now.
int AttendanceDAO::findCurrentSession() {
    const char* sql =
        "SELECT sessionId FROM class_sessions "
        "WHERE sessionDate = date('now') "
        "AND time('now') BETWEEN sessionStartTime AND sessionEndTime "
        "LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return -1;
    }

    int sessionId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        sessionId = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return sessionId;
}

// Mark attendance for a student in a specific class session.

bool AttendanceDAO::markAttendance(int attendanceStudentId, int attendanceSessionId,const std::string& attendanceTime, const std::string& attendanceStatus) {
    const char* sessionSql = "SELECT sessionSubjectId FROM class_sessions WHERE sessionId = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sessionSql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, attendanceSessionId);

    int sessionSubjectId = -1;
    bool sessionFound = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        sessionSubjectId = sqlite3_column_int(stmt, 0);
        sessionFound = true;
    }
    sqlite3_finalize(stmt);
    if (!sessionFound) return false;

    if (!isEnrolled(attendanceStudentId, sessionSubjectId)) return false;

    time_t now = time(nullptr);
    char dateBuf[11]; 
    char timeBuf[9]; 
    strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", localtime(&now));
    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", localtime(&now));

    const char* checkSql = "SELECT COUNT(*) FROM attendance WHERE attendanceStudentId = ? AND attendanceSessionId = ?;";
    
    if (sqlite3_prepare_v2(db, checkSql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, attendanceStudentId);
    sqlite3_bind_int(stmt, 2, attendanceSessionId);
    
    int already = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        already = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (already > 0) return false;

    const char* insertSql = "INSERT INTO attendance (attendanceStudentId, attendanceSessionId, attendanceDate, attendanceTime, attendanceStatus) VALUES (?, ?, ?, ?, ?);";
    
    if (sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, attendanceStudentId);
    sqlite3_bind_int(stmt, 2, attendanceSessionId);
    sqlite3_bind_text(stmt, 3, dateBuf, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, timeBuf, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, attendanceStatus.c_str(), -1, SQLITE_TRANSIENT);
    
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    return ok;
}

std::vector<AttendanceRecord> AttendanceDAO::getAllRecords() {
    std::vector<AttendanceRecord> records;
    std::string sql =
        "SELECT attendanceId, attendanceStudentId, attendanceSessionId, attendanceDate, attendanceTime, attendanceStatus FROM attendance "
        "ORDER BY attendanceDate DESC, attendanceTime DESC;";
        
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AttendanceRecord r;
        r.attendanceId        = sqlite3_column_int(stmt, 0);
        r.attendanceStudentId = sqlite3_column_int(stmt, 1);
        r.attendanceSessionId = sqlite3_column_int(stmt, 2);
        r.attendanceDate      = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        r.attendanceTime      = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        r.attendanceStatus    = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        
        records.push_back(r);
    }
    
    sqlite3_finalize(stmt);
    return records;
}

// Finds lecture attendance per student
std::vector<SubjectAttendancePercentage> AttendanceDAO::getSubjectAttendancePercentage(int subjectId) {
    std::vector<SubjectAttendancePercentage> calculationList;
    

    const char* sql = 
        "SELECT s.studentId, s.studentName, s.studentRollNumber, "
        "       COUNT(a.attendanceId) * 100.0 / cs_cnt.total AS pct "
        "FROM enrollments e "
        "JOIN students s ON s.studentId = e.enrollmentStudentId "
        "JOIN subjects sub ON sub.subjectId = e.enrollmentSubjectId "
        "JOIN ( "
        "  SELECT sessionSubjectId, COUNT(*) AS total "
        "  FROM class_sessions WHERE sessionDate <= date('now') "
        "  GROUP BY sessionSubjectId) cs_cnt ON cs_cnt.sessionSubjectId = sub.subjectId "
        "LEFT JOIN class_sessions cs "
        "  ON cs.sessionSubjectId = sub.subjectId AND cs.sessionDate <= date('now') "
        "LEFT JOIN attendance a "
        "  ON a.attendanceStudentId = s.studentId AND a.attendanceSessionId = cs.sessionId "
        "WHERE sub.subjectId = ? "
        "GROUP BY s.studentId;";
        
    sqlite3_stmt* stmt = nullptr;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return calculationList;
    }
    
    // Lock calculations exclusively to the target course 
    sqlite3_bind_int(stmt, 1, subjectId);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        SubjectAttendancePercentage calculatedRow;
        
        calculatedRow.percentageStudentId = sqlite3_column_int(stmt, 0);
        calculatedRow.percentageStudentName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        calculatedRow.percentageRollNumber = sqlite3_column_int(stmt, 2);
        
        calculatedRow.calculatedPercentage = sqlite3_column_double(stmt, 3);
        
        calculationList.push_back(calculatedRow);
    }
    
    sqlite3_finalize(stmt);
    return calculationList;
}

