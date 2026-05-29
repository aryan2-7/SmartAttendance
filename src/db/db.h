#include <iostream>
#include <string>
#include <sqlite3.h>
#include <vector>
using namespace std;


//for students
struct Student {
    int id=0;
    string name;
    string roll_no;
    string model_path;
    string created_at;
};

//For attendance records
struct attendance_record {
    int id=0;
    int student_id=0;
    string date;
    string time;
};
//Database class to handle database operations
class Database {
    private:
    sqlite3* db;
    void createTable();

    public:
    Database(const string& db_name);
    ~Database();
    void initialize();
    void addStudent(const Student& student);
    void markAttendance(const attendance_record& record);
};
