#include <iostream>
#include <vector>
#include <string>
#include "commands.h"
#include "process.h"
#include "user.h"

bool can_access(const PCB* p) {
    return current_user == "root" || sudo_active ||
           p->owner_user == current_user;
}

void cmd_sudo(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: sudo <su|command>\n";
        return;
    }

    if (current_user == "root") {
        if (args[1] == "su") {
            std::cout << "[INFO] Already root\n";
            return;
        }

        std::vector<std::string> sub(args.begin() + 1, args.end());

        dispatch(sub);

        return;
    }

    int tries = 0;
    std::string psd;

    while (tries < MAX_LOGIN_ATTEMPTS) {
        std::cout << "Password: ";
        std::getline(std::cin, psd);

        if (psd == users["root"].password) break;
        tries++;

        if (tries < MAX_LOGIN_ATTEMPTS)
            std::cout << "Error: incorrect password (" << MAX_LOGIN_ATTEMPTS - tries
                      << " left)\n";
    }

    if (tries >= MAX_LOGIN_ATTEMPTS) {
        std::cout << "Error: too many attempts\n";
        return;
    }

    if (args[1] == "su") {
        current_user = "root";
        std::cout << "[OK] Switched to root\n";
        return;
    }

    sudo_active = true;
    std::vector<std::string> sub(args.begin() + 1, args.end());

    dispatch(sub);
    
    sudo_active = false;
}

void init_users() {
    users["root"].password = "";
    current_user = "root";
}

void first_time_setup() {
    std::cout << "\033[3J\033[2J\033[H" << std::flush;
    
    if (!users["root"].password.empty()) return;

    std::cout << "=== First-time setup ===\n";
    std::cout << "Please set root password: ";

    std::string root_psd;

    while (root_psd.empty()) {
        std::getline(std::cin, root_psd);

        if (root_psd.empty())
            std::cout << "[INFO] Root password cannot be empty\n";
    }

    users["root"].password = root_psd;
    
    std::cout << "[OK] Root password set\n";
}

void cmd_register(const std::vector<std::string>& args) {
    if (current_user != "root") {
        std::cout << "Error: only the root user can register\n";
        return;
    }

    if (args.size() < 3) {
        std::cout << "Usage: register <username> <password>\n";
        return;
    }

    if (users.find(args[1]) != users.end()) {
        std::cout << "Error: user '" << args[1] << "' already exists\n";
        return;
    }

    users[args[1]].password = args[2];

    std::cout << "[OK] Registered user " << args[1] << '\n';
}

void cmd_login(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "Usage: login <username> <password>\n";
        return;
    }

    if (users.find(args[1]) == users.end()) {
        std::cout << "Error: user does not exist\n";
        return;
    }

    if (users[args[1]].locked) {
        std::cout << "[INFO] User " << args[1] << " locked\n";
        return;
    }    

    int tries = 0;
    std::string psd = args[2];

    while (users[args[1]].password != psd) {
        tries++;

        std::cout << "Error: incorrect password\n";

        if (tries >= MAX_LOGIN_ATTEMPTS) {
            users[args[1]].locked = true;

            std::cout << "[INFO] User " << args[1] << " locked\n";
            
            return;
        }

        std::cout << "[INFO] " << MAX_LOGIN_ATTEMPTS - tries
                  << " chances left\n";

        std::cout << "Please enter the password again: ";
        std::getline(std::cin, psd);
    }

    current_user = args[1];

    std::cout << "[OK] User " << args[1] << ", welcome!\n";
}

void cmd_logout(const std::vector<std::string>&) {
    if (current_user == "root") {
        std::cout << "Error: root user cannot log out\n";
        return;
    }

    current_user = "root";

    std::cout << "[OK] Logged out\n";
}