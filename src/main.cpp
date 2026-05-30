#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "commands.h"
#include "process.h"

int main() {
    init_processes();
    std::cout << "\033[3J\033[2J\033[H" << std::flush;
    std::cout << "========================================\n";
    std::cout << "  OS Core Simulator v1.0\n";
    std::cout << "  Type 'help' for commands\n";
    std::cout << "  Type 'exit' to exit\n";
    std::cout << "========================================\n";

    std::string line;
    while(true) {
        std::cout << "os> ";
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        std::istringstream ss(line);
        std::vector<std::string> args;
        
        std::string tmp;
        while (ss >> tmp) args.push_back(tmp);

        if (args[0] == "exit") {
            std::cout << "Bye bye." << std::endl;
            break;
        }

        dispatch(args);
    }

    return 0;
}