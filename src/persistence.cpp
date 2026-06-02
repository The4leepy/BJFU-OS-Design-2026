#include <iostream>
#include <fstream>
#include "persistence.h"
#include "process.h"
#include "scheduler.h"
#include "memory.h"
#include "user.h"

bool auto_load() {
    std::ifstream file("os_state.bin", std::ios::binary);
    if (!file) return false;

    load_scheduler(file);
    load_processes(file);
    load_memory(file);
    load_users(file);

    std::cout << "[INFO] State loaded from os_state.bin\n";
    return true;
}

void save_on_exit() {
    std::ofstream file("os_state.bin", std::ios::binary | std::ios::trunc);
    if (!file) {
        std::cout << "Error: cannot save state\n";
        return;
    }

    save_scheduler(file);
    save_processes(file);
    save_memory(file);
    save_users(file);

    std::cout << "[OK] State saved to os_state.bin\n";
}
