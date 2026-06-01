#pragma once

#include <string>
#include <unordered_map>

struct User {
    std::string password;
    int failed = 0;
    bool locked = false;
};

inline std::unordered_map<std::string, User> users;
inline std::string current_user;
