#include <iostream>
#include <string>
#include <sqlite3.h>
#include <vector>
using namespace std;

struct Student {
    int id=0;
    string name;
    string roll_no;
    string model_path;
    string created_at;
};
struct attendance_record {
    int id=0;
    int student_id;
    string date;
    string time;
};

