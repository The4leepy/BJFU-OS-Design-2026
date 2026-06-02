#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "commands.h"
#include "process.h"
#include "memory.h"
#include "user.h"
#include "scheduler.h"

int main() {
    init_users();
    init_processes();
    init_memory();
    init_scheduler();

    first_time_setup();

    std::cout << "\033[3J\033[2J\033[H" << std::flush;
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