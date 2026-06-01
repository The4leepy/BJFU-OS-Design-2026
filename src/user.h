#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct User {
    std::string password;
    int failed = 0;
    bool locked = false;
};

inline std::unordered_map<std::string, User> users;
inline std::string current_user;

void init_users();

void cmd_register(const std::vector<std::string>&);

void cmd_login(const std::vector<std::string>&);

void cmd_logout(const std::vector<std::string>&);
