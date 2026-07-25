#include "UserDAO.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <random>
#include "sha256.h"


// Constructor
UserDAO::UserDAO(sqlite3* databaseConnection) : db(databaseConnection) {}


// Helper to generate a random, secure 16-character salt per user
std::string UserDAO::generateSalt() {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string salt = "";
    
    static std::random_device rd;
    static std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, sizeof(charset) - 2);

    for (int i = 0; i < 16; ++i) {
        salt += charset[distribution(generator)];
    }
    return salt;
}


// Uses header-only SHA256 class to hash salt + password
std::string UserDAO::hashPassword(const std::string& userSalt, const std::string& userPassword) {
    return SHA256::hash(userSalt + userPassword);
}


// Creates a new user into the database with a unique salt
bool UserDAO::createUser(const std::string& username, const std::string& password) {
    const char* sql = "INSERT INTO users (username, password, salt) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
   
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
   
    // Generate a unique crypto salt for user
    std::string salt = generateSalt();
    // Hash the password combined with unique salt
    std::string hashedPassword = hashPassword(salt, password);
   
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, salt.c_str(), -1, SQLITE_TRANSIENT);
   
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}


// Validate login
bool UserDAO::checkLogin(const std::string& username, const std::string& password) {
    const char* sql = "SELECT password, salt FROM users WHERE username = ?;";
    sqlite3_stmt* stmt = nullptr;
   
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
   
    std::string storedHash = "";
    std::string storedSalt = "";
    bool userExists = false;
   
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        userExists = true;
        storedHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        storedSalt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    }
    sqlite3_finalize(stmt);
   
    if (!userExists) return false;
   
    // Re-hash the password with the user's unique salt from the database
    std::string inputHash = hashPassword(storedSalt, password);

    return (inputHash == storedHash);
}

