#pragma once
#include "ultron.h"
#include "logger.h"

// =======================================================
//  AUTH MANAGER  –  Task 2: User Login & Authentication
// =======================================================
class AuthManager {
private:
    // username → password (plain text for simulation)
    std::unordered_map<std::string, std::string> users;
    std::string currentUser;
    bool        loggedIn;
    Logger&     logger;

public:
    AuthManager(Logger& log) : loggedIn(false), logger(log) {
        // Seed default accounts
        users["root"]  = "root123";
        users["admin"]  = "admin123";
        users["zaran"] = "zaran123";
    }

    // -- Register new user ------------------------------
    bool registerUser(const std::string& uname, const std::string& pass) {
        if (users.count(uname)) {
            logger.warn("AUTH", "Register failed – user '" + uname + "' already exists.");
            return false;
        }
        users[uname] = pass;
        logger.info("AUTH", "New user registered: " + uname);
        return true;
    }

    // -- Login ------------------------------------------
    bool login(const std::string& uname, const std::string& pass) {
        auto it = users.find(uname);
        if (it == users.end()) {
            logger.error("AUTH", "Login failed – unknown user: " + uname);
            return false;
        }
        if (it->second != pass) {
            logger.warn("AUTH", "Login failed – wrong password for: " + uname);
            return false;
        }
        currentUser = uname;
        loggedIn    = true;
        logger.info("AUTH", "User '" + uname + "' logged in successfully.");
        return true;
    }

    // -- Logout -----------------------------------------
    void logout() {
        if (loggedIn) {
            logger.info("AUTH", "User '" + currentUser + "' logged out.");
            currentUser.clear();
            loggedIn = false;
        }
    }

    // -- Change password --------------------------------
    bool changePassword(const std::string& oldPass, const std::string& newPass) {
        if (!loggedIn) return false;
        if (users[currentUser] != oldPass) {
            logger.warn("AUTH", "passwd: incorrect current password.");
            return false;
        }
        users[currentUser] = newPass;
        logger.info("AUTH", "Password changed for: " + currentUser);
        return true;
    }

    // -- Delete user (root only) ------------------------
    bool deleteUser(const std::string& target) {
        if (currentUser != "root") {
            logger.error("AUTH", "Permission denied: only root can delete users.");
            return false;
        }
        if (!users.count(target)) {
            logger.warn("AUTH", "deluser: user '" + target + "' not found.");
            return false;
        }
        users.erase(target);
        logger.info("AUTH", "User '" + target + "' deleted by root.");
        return true;
    }

    // -- List users -------------------------------------
    void listUsers() const {
        std::cout << CYAN << "  Registered users:\n" << RESET;
        for (const auto& kv : users)
            std::cout << "    • " << kv.first << "\n";
    }

    bool        isLoggedIn()    const { return loggedIn; }
    std::string getCurrentUser() const { return currentUser; }
    bool        isRoot()         const { return currentUser == "root"; }
};
