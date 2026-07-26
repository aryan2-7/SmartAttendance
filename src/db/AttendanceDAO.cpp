#include "AttendanceDAO.h"
#include <iostream>
#include <ctime>

// Constructor
AttendanceDAO::AttendanceDAO(sqlite3* databaseConnection) : db(databaseConnection) {}

// Parses "HH:MM:SS" (or "HH:MM") into total seconds since midnight.
// Returns -1 if the string can't be parsed.
static int parseTimeToSeconds(const std::string& timeStr) {
    int h = 0, m = 0, s = 0;
    int fields = sscanf(timeStr.c_str(), "%d:%d:%d", &h, &m, &s);
    if (fields < 2) return -1;
    return h * 3600 + m * 60 + s;
}

// A check-in is late if it lands more than LATE_THRESHOLD_MINUTES after the
// session's scheduled start time.
bool isLateArrival(const std::string& attendanceTime, const std::string& sessionStartTime) {
    int attendanceSeconds = parseTimeToSeconds(attendanceTime);
    int startSeconds = parseTimeToSeconds(sessionStartTime);
    if (attendanceSeconds < 0 || startSeconds < 0) return false;
    return (attendanceSeconds - startSeconds) > (LATE_THRESHOLD_MINUTES * 60);
}


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
        "WHERE sessionDate = date('now', 'localtime') "
        "AND time('now', 'localtime') BETWEEN sessionStartTime AND sessionEndTime "
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
    const char* sessionSql = "SELECT sessionSubjectId, sessionDate FROM class_sessions WHERE sessionId = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sessionSql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, attendanceSessionId);

    int sessionSubjectId = -1;
    std::string sessionDate;
    bool sessionFound = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        sessionSubjectId = sqlite3_column_int(stmt, 0);

        const char* dateText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (dateText) {
            sessionDate = dateText;
        }
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
    std::string finalTime = attendanceTime.empty() ? timeBuf : attendanceTime;

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
    sqlite3_bind_text(stmt, 4, finalTime.c_str(), -1, SQLITE_TRANSIENT);
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
        
    sqlite3_stmt* stmt=nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    return records; 
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AttendanceRecord r;
        r.attendanceId        = sqlite3_column_int(stmt, 0);
        r.attendanceStudentId = sqlite3_column_int(stmt, 1);
        r.attendanceSessionId = sqlite3_column_int(stmt, 2);
        const char* dText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const char* tText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        const char* sText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        if (dText) r.attendanceDate = dText;
        if (tText) r.attendanceTime = tText;
        if (sText) r.attendanceStatus = sText;

        records.push_back(r);
    }
    
    sqlite3_finalize(stmt);
    return records;
}

// Joined attendance rows (student name/roll + session date/time) for a date
// range, optionally scoped to a single subject via the class_sessions link.
std::vector<AttendanceDisplayRecord> AttendanceDAO::getDisplayRecords(
    const std::string& startDate, const std::string& endDate, int subjectId) {
    std::vector<AttendanceDisplayRecord> records;

    std::string sql =
        "SELECT s.studentId, s.studentName, s.studentRollNumber, "
        "cs.sessionDate, a.attendanceTime, a.attendanceStatus, cs.sessionStartTime "
        "FROM attendance a "
        "JOIN students s ON s.studentId = a.attendanceStudentId "
        "JOIN class_sessions cs ON cs.sessionId = a.attendanceSessionId "
        "WHERE cs.sessionDate BETWEEN ? AND ? ";
    if (subjectId >= 0) {
        sql += "AND cs.sessionSubjectId = ? ";
    }
    sql += "ORDER BY cs.sessionDate DESC, a.attendanceTime DESC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return records;
    }

    sqlite3_bind_text(stmt, 1, startDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, endDate.c_str(), -1, SQLITE_TRANSIENT);
    if (subjectId >= 0) {
        sqlite3_bind_int(stmt, 3, subjectId);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AttendanceDisplayRecord r;
        r.displayStudentId = sqlite3_column_int(stmt, 0);
        const char* nameText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (nameText) r.displayStudentName = nameText;
        r.displayRollNumber = sqlite3_column_int(stmt, 2);
        const char* dateText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        if (dateText) r.displaySessionDate = dateText;
        const char* timeText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        if (timeText) r.displayAttendanceTime = timeText;
        const char* statusText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        if (statusText) r.displayAttendanceStatus = statusText;
        const char* startText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        if (startText) r.displaySessionStartTime = startText;
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
        "CASE WHEN COALESCE(cs_cnt.total, 0) = 0 THEN 0.0 "
        "  ELSE COUNT(a.attendanceId) * 100.0 / cs_cnt.total END AS pct "
        "FROM enrollments e "
        "JOIN students s ON s.studentId = e.enrollmentStudentId "
        "JOIN subjects sub ON sub.subjectId = e.enrollmentSubjectId "
        "JOIN ( "
        "  SELECT sessionSubjectId, COUNT(*) AS total "
        "  FROM class_sessions WHERE sessionDate <= date('now', 'localtime') "
        "  GROUP BY sessionSubjectId) cs_cnt ON cs_cnt.sessionSubjectId = sub.subjectId "
        "LEFT JOIN class_sessions cs "
        "  ON cs.sessionSubjectId = sub.subjectId AND cs.sessionDate <= date('now', 'localtime') "
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

        const char* nameText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (nameText) {
            calculatedRow.percentageStudentName = nameText;
        }
        
        calculatedRow.percentageRollNumber = sqlite3_column_int(stmt, 2);
        calculatedRow.calculatedPercentage = sqlite3_column_double(stmt, 3);
        calculationList.push_back(calculatedRow);
    }
    
    sqlite3_finalize(stmt);
    return calculationList;
}

// Counts distinct (student, subject) pairs where the computed attendance %
// falls below that subject's configured minimum attendance threshold.
int AttendanceDAO::countStudentsBelowMinimumAttendance() {
    const char* sql =
        "SELECT COUNT(*) FROM ( "
        "  SELECT s.studentId, sub.subjectMinAttendance, "
        "    CASE WHEN COALESCE(cs_cnt.total, 0) = 0 THEN 0.0 "
        "      ELSE COUNT(a.attendanceId) * 100.0 / cs_cnt.total END AS pct "
        "  FROM enrollments e "
        "  JOIN students s ON s.studentId = e.enrollmentStudentId "
        "  JOIN subjects sub ON sub.subjectId = e.enrollmentSubjectId "
        "  JOIN ( "
        "    SELECT sessionSubjectId, COUNT(*) AS total "
        "    FROM class_sessions WHERE sessionDate <= date('now', 'localtime') "
        "    GROUP BY sessionSubjectId) cs_cnt ON cs_cnt.sessionSubjectId = sub.subjectId "
        "  LEFT JOIN class_sessions cs "
        "    ON cs.sessionSubjectId = sub.subjectId AND cs.sessionDate <= date('now', 'localtime') "
        "  LEFT JOIN attendance a "
        "    ON a.attendanceStudentId = s.studentId AND a.attendanceSessionId = cs.sessionId "
        "  GROUP BY s.studentId, sub.subjectId "
        ") AS perSubject "
        "WHERE pct < subjectMinAttendance;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

