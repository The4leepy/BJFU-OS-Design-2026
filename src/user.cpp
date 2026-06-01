#include <iostream>
#include <vector>
#include <string>
#include "user.h"

void init_users() {
    users["root"].password = "1";
    current_user = "root";
}

void cmd_register(const std::vector<std::string>& args) {
    if (current_user != "root") {
        std::cout << "ERROR: Only the root user can register\n";
        return;
    }

    if (args.size() < 3) {
        std::cout << "Usage: cmd_register <username> <password>\n";
        return;
    }

    if (users.find(args[1]) != users.end()) {
        std::cout << "Error: already exists\n";
        return;
    }

    users[args[1]].password = args[2];

    current_user = args[1];

    std::cout << "[OK] Registered user " << args[1] << '\n';
}

void cmd_login(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "Usage: cmd_login <username> <password>\n";
        return;
    }

    if (users.find(args[1]) == users.end()) {
        std::cout << "Error: User does not exist\n";
        return;
    }

    if (users[args[1]].locked) {
        std::cout << "\nInfo: User " << args[1] << " locked\n\n";
        return;
    }    

    if (users[args[1]].password != args[2]) {
        users[args[1]].failed++;

        std::cout << "Error: Incorrect password\n";

        if (users[args[1]].failed == 3) {
            users[args[1]].locked = true;
            std::cout << "\nInfo: User " << args[1] << " locked\n\n";
            return;
        }

        std::cout << "\nInfo: " << 3 - users[args[1]].failed << " chances left\n\n";

        return;
    }

    current_user = args[1];

    std::cout << "[OK] User " << args[1] << ", welcome!\n";
}

void cmd_logout(const std::vector<std::string>&) {
    if (current_user == "root") {
        std::cout << "Error: Root user cannot log out\n";
        return;
    }

    current_user = "root";

    std::cout << "[OK] Log out\n";
}