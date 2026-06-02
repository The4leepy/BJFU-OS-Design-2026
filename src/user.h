#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct User {
    std::string password;
    bool locked = false;
};

constexpr int MAX_LOGIN_ATTEMPTS = 3;

inline std::unordered_map<std::string, User> users;
inline std::string current_user;
inline bool sudo_active = false;

struct PCB;

bool can_access(const PCB* p);

void init_users();

void first_time_setup();

void cmd_register(const std::vector<std::string>&);

void cmd_login(const std::vector<std::string>&);

void cmd_logout(const std::vector<std::string>&);

void cmd_sudo(const std::vector<std::string>&);
