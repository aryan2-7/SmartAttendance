#pragma once
#include <string>
#include "sqlite3.h"

struct UserRecord {
    int userId;
    std::string userUsername;
    std::string userPasswordHash;
    std::string userSalt;
};

class UserDAO {
public:
    explicit UserDAO(sqlite3* db);

    bool checkLogin(const std::string& userUsername, const std::string& userPassword);
    bool createUser(const std::string& userUsername, const std::string& userPassword);

private:
    sqlite3* db;

    std::string generateSalt();
    std::string hashPassword(const std::string& userSalt, const std::string& userPassword);
};