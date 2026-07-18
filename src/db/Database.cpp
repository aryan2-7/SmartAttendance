#include "Database.h"
#include <iostream>


//Constructor to initialize the database connection
Database::Database(const std::string& dbPath) : db(nullptr) {
    int rc=sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << "\n";
        db = nullptr;
    } else {
        // Enforce foreign key constraints for table relationships
        execute("PRAGMA foreign_keys = ON;");
    }
}

//Destructor to close the database connection
Database::~Database() {
    if (db) {
        sqlite3_close(db);
    }
}

// Exposes the raw connection so DAOs can be constructed with it
sqlite3* Database::getConnection() const {
    return db;
}

// A helper function to run raw SQL commands 
bool Database::execute(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << "\n";
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}
