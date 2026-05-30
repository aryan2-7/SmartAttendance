#include <iostream>
#include <string>
#include <sqlite3.h>
#include "db.h"
using namespace std;
static int createDB(const char* s);
static int createTable(const char* s);

int main(){
    const char* db_name = "C:\\SmartAttendance\\resources\\attendance.db";
    createDB(db_name);
    createTable(db_name);
    return 0;
}

static int createDB(const char* s){
    sqlite3* DB;
    int exit = 0;
    exit = sqlite3_open(s, &DB);
    sqlite3_close(DB);
    return 0;
}

static int createTable(const char* s){
    sqlite3* DB;
    string sql = "CREATE TABLE IF NOT EXISTS Students("
                 "ID INTEGER PRIMARY KEY AUTOINCREMENT," 
                 "NAME TEXT NOT NULL," 
                 "ROLL_NO TEXT NOT NULL," 
                 "MODEL_PATH TEXT NOT NULL,"
                 "CREATED_AT TEXT NOT NULL);"
                 "CREATE TABLE IF NOT EXISTS attendance_records ("
        "    id          INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    student_id  INTEGER NOT NULL,"
        "    date        TEXT NOT NULL,"
        "    time        TEXT NOT NULL,"
        "    FOREIGN KEY (student_id) REFERENCES students(id) ON DELETE CASCADE"
        ");";

    try{
        int exit = 0;
        exit = sqlite3_open(s, &DB);
        char* messageError;
        exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messageError);
        if (exit != SQLITE_OK) {
            cerr << "Error creating table " << endl;
            sqlite3_free(messageError);
        } else {
            cout << "Table created successfully" << endl;
            sqlite3_close(DB);
        }
    }
    catch (const exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }  
    return 0;
}

