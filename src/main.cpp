#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include "commands.h"
#include "process.h"
#include "memory.h"
#include "user.h"
#include "scheduler.h"
#include "persistence.h"

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    init_users();
    init_processes();
    init_memory();
    init_scheduler();
    
    if (!auto_load()) first_time_setup();

    start_background();
    if (try_lock_master()) {
        is_master = true;
        std::cout << "[INFO] Running as master instance\n";
    } else {
        is_master = false;
        std::cout << "[INFO] Running as viewer (another instance is master)\n";
    }
    std::cout << "========================================\n";
    std::cout << "  OS Core Simulator v1.0\n";
    std::cout << "  Type 'help' for commands\n";
    std::cout << "========================================\n";

    std::string line;

    while (!exit_requested) {
        std::cout << current_user << "@os> ";
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        std::istringstream ss(line);
        std::vector<std::string> args;

        std::string tmp;
        while (ss >> tmp) args.emplace_back(std::move(tmp));

        if (args.empty()) continue;

        dispatch(args);
    }

    return 0;
}