#pragma once

#include <vector>
#include <string>
#include <fstream>
#include "common.h"

struct Proc_Mem_Blo {
    int base;
    int size;
    bool is_swaped = 0;
};

int get_tol_size(const std::vector<Proc_Mem_Blo>&);

struct PCB {
    int pid;
    int ppid;
    std::string name;
    Proc_State state;
    int priority;
    int current_queue;

    std::string owner_user;

    std::vector<Proc_Mem_Blo> mem;

    int cpu_time;

    std::vector<int> child;

    std::string get_value(const std::string& tar) const {
        if (tar == "pid")      return "PID:      " + std::to_string(pid);
        if (tar == "ppid")     return "PPID:     " + std::to_string(ppid);
        if (tar == "name")     return "Name:     " + name;
        if (tar == "state") {
            const char* s = "UNKNOWN";
            switch (state) {
                case Proc_State::READY:     s = "READY";     break;
                case Proc_State::RUNNING:   s = "RUNNING";   break;
                case Proc_State::BLOCKED:   s = "BLOCKED";   break;
                case Proc_State::SUSPENDED: s = "SUSPENDED"; break;
                case Proc_State::ZOMBIE:    s = "ZOMBIE";    break;
            }
            return "State:    " + std::string(s);
        }
        if (tar == "priority") return "Priority: " + std::to_string(priority);
        if (tar == "mem")     return "Memory:   " + std::to_string(get_tol_size(mem)) + " KB";
        if (tar == "cpu")     return "CPU Time: " + std::to_string(cpu_time);
        if (tar == "child") {
            std::string tmp;
            for (int c : child) tmp += std::to_string(c) + " ";
            if (tmp.empty()) tmp = "(none)";
            return "Children: " + tmp;
        }
        return "Invalid member: " + tar;
    }
};

void init_processes();

PCB* find_pcb(int pid);

void cmd_create(const std::vector<std::string>&);

void cmd_show(const std::vector<std::string>&);

void cmd_list(const std::vector<std::string>&);

void cmd_renice(const std::vector<std::string>&);

void cmd_block(const std::vector<std::string>&);

void cmd_wakeup(const std::vector<std::string>&);

void cmd_suspend(const std::vector<std::string>&);

void cmd_resume(const std::vector<std::string>&);

void cmd_ptree(const std::vector<std::string>&);

void cmd_kill(const std::vector<std::string>&);

void save_processes(std::ofstream&);

void load_processes(std::ifstream&);