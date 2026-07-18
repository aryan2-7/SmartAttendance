#pragma once
#include <string>
#include "sqlite3.h"

class Database {
public:
    explicit Database(const std::string& dbPath);
    ~Database();

    bool initializeTables();
    sqlite3* getConnection() const;
    bool execute(const std::string& sql);

private:
    sqlite3* db;
};