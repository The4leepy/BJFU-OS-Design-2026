#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <functional>
#include "commands.h"
#include "process.h"
#include "memory.h"
#include "user.h"
#include "scheduler.h"

using Cmd_handler = std::function<void(std::vector<std::string>)>;

struct CmdInfo {
    Cmd_handler handler;
    std::string desc;
    std::string category;
};

static std::unordered_map<std::string, CmdInfo> Cmd;

static void cmd_exit(const std::vector<std::string>&) {
    std::cout << "Goodbye\n";
    exit_requested = true;
}

static void cmd_clear(const std::vector<std::string>&) {
    std::cout << "\033[3J\033[2J\033[H" << std::flush;
    std::cout << "========================================\n";
    std::cout << "  OS Core Simulator v1.0\n";
    std::cout << "  Type 'help' for commands\n";
    std::cout << "  Type 'exit' to exit\n";
    std::cout << "========================================\n";
}

static void cmd_help(const std::vector<std::string>& args) {
    std::string filter = "";

    if (args.size() >= 2 && args[1] != "all")
        filter = args[1];

    if (!filter.empty())
        filter[0] = std::toupper(filter[0]);

    std::map<std::string, std::vector<std::pair<std::string, std::string>>> groups;
    for (auto& [name, info] : Cmd)
        if (filter.empty() || info.category == filter)
            groups[info.category].emplace_back(name, info.desc);

    std::map<std::string, std::string> titles = {
        {"User",      "User Commands"},
        {"Process",   "Process Commands"},
        {"Memory",    "Memory Commands"},
        {"Scheduler", "Scheduler Commands"},
        {"System",    "System Commands"},
    };

    if (groups.empty() && !filter.empty()) {
        std::cout << "No commands in category: " << args[1] << "\n";
        return;
    }

    for (auto& [cat, cmds] : groups) {
        std::cout << "\n=== " << titles[cat] << " ===\n";
        for (auto& [name, desc] : cmds)
            printf("  %-18s - %s\n", name.c_str(), desc.c_str());
    }

}

static struct _Init_Cmd {
    _Init_Cmd() {
        // User
        Cmd["register"] = {cmd_register, "Register a new user",       "User"};
        Cmd["login"]    = {cmd_login,    "Login to the system",       "User"};
        Cmd["logout"]   = {cmd_logout,   "Logout from the system",    "User"};
        Cmd["sudo"]     = {cmd_sudo,     "Execute command as root",    "User"};
        // System
        Cmd["help"]  = {cmd_help,  "Show help [process|memory|system]", "System"};
        Cmd["clear"] = {cmd_clear, "Clear terminal screen",          "System"};
        Cmd["exit"]  = {cmd_exit,  "Exit the simulator",             "System"};
        // Process
        Cmd["create"] = {cmd_create, "Create a new process",         "Process"};
        Cmd["show"]   = {cmd_show,   "Show process details",         "Process"};
        Cmd["list"]   = {cmd_list,   "List all processes",           "Process"};
        Cmd["renice"]     = {cmd_renice,     "Change process priority",      "Process"};
        Cmd["block"]  = {cmd_block,  "Block a process",              "Process"};
        Cmd["wakeup"] = {cmd_wakeup, "Wake up a blocked process",    "Process"};
        Cmd["suspend"]    = {cmd_suspend,"Suspend a process",            "Process"};
        Cmd["resume"]     = {cmd_resume, "Resume a suspended process",   "Process"};
        Cmd["ptree"]      = {cmd_ptree,      "Show process tree",            "Process"};
        Cmd["kill"]   = {cmd_kill,   "Kill a process",               "Process"};
        // Memory
        Cmd["set_alloc_algo"] = {cmd_set_alloc_algo, "Set allocation algorithm", "Memory"};
        Cmd["show_mem"]       = {cmd_show_mem,       "Show memory map",          "Memory"};
        Cmd["mem_stat"]       = {cmd_mem_stat,       "Show memory statistics",   "Memory"};
        Cmd["alloc"]          = {cmd_alloc,          "Allocate memory to process","Memory"};
        Cmd["free_mem"]       = {cmd_free_mem,       "Free process memory",       "Memory"};
        Cmd["compact"]        = {cmd_compact,        "Compact memory",             "Memory"};
        Cmd["pgfault"]        = {cmd_pgfault,        "Simulate a page fault",      "Memory"};
        Cmd["swap_out"]       = {cmd_swap_out,       "Swap out process memory",    "Memory"};
        // Scheduler
        Cmd["step"]            = {cmd_step,            "Execute one scheduler tick", "Scheduler"};
    }
} _init_Cmd;

void dispatch(const std::vector<std::string>& args) {
    if (args.empty()) return;
    auto it = Cmd.find(args[0]);
    if (it != Cmd.end()) {
        it->second.handler(args);
    } else {
        std::cout << "Error: unknown command '" << args[0] << "'\n";
    }
}