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
