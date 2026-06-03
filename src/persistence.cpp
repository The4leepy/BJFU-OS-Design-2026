#include <iostream>
#include <fstream>
#include <cstdio>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "persistence.h"
#include "process.h"
#include "scheduler.h"
#include "memory.h"
#include "user.h"

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

void save_on_exit() {
    std::lock_guard<std::mutex> lock(sched_mtx);
    std::ofstream file("os_state.bin.tmp", std::ios::binary | std::ios::trunc);
    if (!file) {
        std::cout << "Error: cannot save state\n";
        return;
    }

    save_scheduler(file);
    save_processes(file);
    save_memory(file);
    save_users(file);
    file.close();

    if (std::rename("os_state.bin.tmp", "os_state.bin") != 0) {
        std::cout << "Error: cannot save state\n";
        return;
    }
    std::cout << "[OK] State saved to os_state.bin\n";
}

void auto_save() {
    std::lock_guard<std::mutex> lock(sched_mtx);
    std::ofstream file("os_state.bin.tmp", std::ios::binary | std::ios::trunc);
    if (!file) return;
    save_scheduler(file);
    save_processes(file);
    save_memory(file);
    save_users(file);
    file.close();
    std::rename("os_state.bin.tmp", "os_state.bin");
}

bool auto_load() {
    std::cout << "\033[3J\033[2J\033[H" << std::flush;
    std::ifstream file("os_state.bin", std::ios::binary);
    if (!file) return false;
    
    std::cout << "Load previous state? (y/n): ";
    std::string ans;
    std::getline(std::cin, ans);

    if (ans == "n" || ans == "N") return false;

    load_scheduler(file);
    load_processes(file);
    load_memory(file);
    load_users(file);

    return true;
}

static time_t last_mtime = 0;

void check_reload() {
    struct stat st;
    if (stat("os_state.bin", &st) != 0) return;
    if (st.st_mtime != last_mtime) {
        last_mtime = st.st_mtime;
        auto_load();
    }
}
