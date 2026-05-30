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

//Creates database at db_path, and table if not exists, 
//Called when application starts
bool initializeDatabase(const string& db_path);

//Inserts a student record into the database, called when a new student is added
//Returns new row id on success, -1 on failure
int addStudent(const string& name, const string& roll_no, const string& model_path);


//Returns true if attendance for the student_id on the given date is already marked, false otherwise
bool isAlreadyMarked(int student_id, const string& date);

//Marks attendance for the student_id on the given date and time
//Internally calls isAlreadyMarked to prevent duplicate entries, returns true on successful marking, false if already marked or on failure
bool markAttendance(int student_id, const string& date, const string& time);

//Returns a vector of attendance records for the given date, empty vector if no records or on failure
vector<attendance_record> getRecords(const string& date);

// Closes the database connection. Call at application shutdown.
void closeDatabase();

