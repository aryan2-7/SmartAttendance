#pragma once
#include <string>
#include <vector>
#include "sqlite3.h"

struct SubjectRecord {
    int subjectId;
    std::string subjectCode;
    std::string subjectName;
    int subjectSemester;
    std::string subjectDepartment;
    int subjectMinAttendance;
};

struct ClassSessionRecord {
    int sessionId;
    int sessionSubjectId;
    std::string sessionDate;
    std::string sessionStartTime;
    std::string sessionEndTime;
    std::string sessionRoom;
    std::string sessionTopic;
};

class SubjectDAO {
public:
    explicit SubjectDAO(sqlite3* db);

    bool createSubject(const std::string& subjectCode, const std::string& subjectName, int subjectSemester, const std::string& subjectDepartment, int subjectMinAttendance = 80);
    std::vector<SubjectRecord> getAllSubjects();
    bool createClassSession(int sessionSubjectId, const std::string& sessionDate, const std::string& sessionStartTime, const std::string& sessionEndTime, const std::string& sessionRoom = "", const std::string& sessionTopic = "");
    std::vector<ClassSessionRecord> getSessionsForSubject(int subjectId);
    bool deleteClassSession(int sessionId);

private:
    sqlite3* db;
};