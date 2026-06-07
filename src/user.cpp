#include <iostream>
#include <vector>
#include <string>
#include "commands.h"
#include "process.h"
#include "user.h"
#include "persistence.h"

bool can_access(const PCB* p) {
    return current_user == "root" || sudo_active ||
           p->owner_user == current_user;
}
bool user_changed = false;

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
    if (args.size() < 2) {
        std::cout << "Usage: login <username>\n";
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
    std::string psd;

    while (tries < MAX_LOGIN_ATTEMPTS) {
        std::cout << "Password: ";
        std::getline(std::cin, psd);

        if (users[args[1]].password == psd) break;

        tries++;
        std::cout << "Error: incorrect password\n";

        if (tries >= MAX_LOGIN_ATTEMPTS) {
            users[args[1]].locked = true;
            std::cout << "[INFO] User " << args[1] << " locked\n";
            return;
        }

        std::cout << "[INFO] " << MAX_LOGIN_ATTEMPTS - tries
                  << " chances left\n";
    }

    current_user = args[1];
    user_changed = true;

    std::cout << "[OK] User " << args[1] << ", welcome!\n";
}

void cmd_logout(const std::vector<std::string>&) {
    if (current_user == "root") {
        std::cout << "Error: root user cannot log out\n";
        return;
    }

    std::string log_out_name = current_user;

    std::cout << "[OK] Logged out\n";
    std::cout << "Existing users: ";
    for (auto& [name, _] : users) std::cout << name << ' ';
    std::cout << '\n';

    std::string log_name = "";
    user_changed = false;

    while (!user_changed) {
        std::cout << "Please enter the username you want to log in with: ";

        std::getline(std::cin, log_name);
        cmd_login({"login", log_name});
    }
    
}

void save_users(std::ofstream& f) {
    int sz = static_cast<int>(users.size());
    f.write(reinterpret_cast<const char*>(&sz), sizeof(sz));

    for (const auto& [name, u] : users) {
        write_str(f, name);
        write_str(f, u.password);
        char locked_flag = u.locked ? 1 : 0;
        f.write(&locked_flag, 1);
    }
    write_str(f, current_user);
    char sudo_flag = sudo_active ? 1 : 0;
    f.write(&sudo_flag, 1);
}

void load_users(std::ifstream& f) {
    users.clear();
    int sz = 0;
    f.read(reinterpret_cast<char*>(&sz), sizeof(sz));
    
    for (int i = 0; i < sz; i++) {
        std::string name = read_str(f);
        std::string pass = read_str(f);
        char locked_flag;
        f.read(&locked_flag, 1);
        users[name] = {pass, locked_flag != 0};
    }
    current_user = read_str(f);
    char sudo_flag;
    f.read(&sudo_flag, 1);
    sudo_active = (sudo_flag != 0);
}

void cmd_unlock(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: unlock <username>\n";
        return;
    }

    if (current_user != "root" && !sudo_active) {
        std::cout << "Error: permission denied\n";
        return;
    }

    if (users.find(args[1]) == users.end()) {
        std::cout << "Error: user does not exist\n";
        return;
    }

    users[args[1]].locked = false;

    std::cout << "[OK] user " << args[1] << " unlocked\n";
}

void cmd_show_users(const std::vector<std::string>&) {
    if (users.empty()) {
        std::cout << "No registered users.\n";
        return;
    }
    std::cout << std::left
              << std::setw(16) << "USERNAME"
              << std::setw(10) << "STATUS" << '\n'
              << std::string(26, '-') << '\n';
    for (const auto& [name, u] : users) {
        std::cout << std::left
                  << std::setw(16) << name
                  << std::setw(10)
                  << (name == current_user ? "online" :
                      (u.locked ? "locked" : "offline"))
                  << '\n';
    }
}