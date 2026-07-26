#pragma once
#include <string>
#include <vector>
#include "sqlite3.h"

struct AttendanceRecord {
    int attendanceId;
    int attendanceStudentId;
    int attendanceSessionId;
    std::string attendanceDate;
    std::string attendanceTime;
    std::string attendanceStatus;
};

struct AttendanceDisplayRecord {
    int displayStudentId;
    std::string displayStudentName;
    int displayRollNumber;
    std::string displaySessionDate;
    std::string displayAttendanceTime;
    std::string displayAttendanceStatus;
    std::string displaySessionStartTime; // scheduled start time (HH:MM:SS) of the session, used for late calc
};

struct SubjectAttendancePercentage {
    int percentageStudentId;
    std::string percentageStudentName;
    int percentageRollNumber;
    double calculatedPercentage;
};

// A student is considered "late" if they check in more than this many
// minutes after the session's scheduled start time.
inline constexpr int LATE_THRESHOLD_MINUTES = 10;

// Returns true if attendanceTime ("HH:MM:SS") is more than
// LATE_THRESHOLD_MINUTES after sessionStartTime ("HH:MM:SS").
// Falls back to false if either time string can't be parsed.
bool isLateArrival(const std::string& attendanceTime, const std::string& sessionStartTime);

class AttendanceDAO {
public:
    explicit AttendanceDAO(sqlite3* db);

    bool markAttendance(int attendanceStudentId, int attendanceSessionId, const std::string& attendanceTime, const std::string& attendanceStatus = "present");

    std::vector<AttendanceRecord> getAllRecords();

    // Joined, human-readable attendance rows for a date range, optionally
    // filtered to one subject (pass subjectId < 0 for all subjects).
    std::vector<AttendanceDisplayRecord> getDisplayRecords(
        const std::string& startDate, const std::string& endDate, int subjectId = -1);

    std::vector<SubjectAttendancePercentage> getSubjectAttendancePercentage(int subjectId);

    // Counts distinct students whose computed attendance % is below their
    // subject's configured minimum, across ALL subjects. Used for the
    // dashboard "below minimum" alert banner.
    int countStudentsBelowMinimumAttendance();

    // Returns the sessionId of the class session happening right now, or -1 if none.
    int findCurrentSession();

    // Checks the enrollments table for a matching student/subject pair.
    bool isEnrolled(int studentId, int subjectId);

private:
    sqlite3* db;
};