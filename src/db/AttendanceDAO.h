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
};

struct SubjectAttendancePercentage {
    int percentageStudentId;
    std::string percentageStudentName;
    int percentageRollNumber;
    double calculatedPercentage;
};

class AttendanceDAO {
public:
    explicit AttendanceDAO(sqlite3* db);

    bool markAttendance(int attendanceStudentId, int attendanceSessionId, const std::string& attendanceTime, const std::string& attendanceStatus = "present");

    std::vector<AttendanceRecord> getAllRecords();

    std::vector<SubjectAttendancePercentage> getSubjectAttendancePercentage(int subjectId);

    // Returns the sessionId of the class session happening right now, or -1 if none.
    int findCurrentSession();

    // Checks the enrollments table for a matching student/subject pair.
    bool isEnrolled(int studentId, int subjectId);

private:
    sqlite3* db;
};