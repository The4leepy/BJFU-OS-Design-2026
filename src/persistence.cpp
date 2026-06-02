#include <iostream>
#include <fstream>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
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

static int lock_fd = -1;

bool try_lock_master() {
    lock_fd = open("os_state.lock", O_CREAT | O_RDWR, 0644);
    if (lock_fd < 0) return false;
    if (flock(lock_fd, LOCK_EX | LOCK_NB) != 0) {
        close(lock_fd);
        lock_fd = -1;
        return false;
    }
    return true;
}
