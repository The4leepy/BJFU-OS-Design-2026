#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <functional>
#include "commands.h"
#include "process.h"
#include "memory.h"

using Cmd_handler = std::function<void(std::vector<std::string>)>;

struct CmdInfo {
    Cmd_handler handler;
    std::string desc;
    std::string category;
};

static std::map<std::string, CmdInfo> Cmd;

static void cmd_clear(const std::vector<std::string>) {
    std::cout << "\033[3J\033[2J\033[H" << std::flush;
    std::cout << "========================================\n";
    std::cout << "  OS Core Simulator v1.0\n";
    std::cout << "  Type 'help' for commands\n";
    std::cout << "  Type 'exit' to exit\n";
    std::cout << "========================================\n";
}

static void cmd_help(const std::vector<std::string>) {
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> groups;
    for (auto& [name, info] : Cmd)
        groups[info.category].
        emplace_back(std::pair<std::string , std::string>{name, info.desc});

    std::map<std::string, std::string> titles = {
        {"Process", "Process Commands"},
        {"Memory",  "Memory Commands"},
        {"System",  "System Commands"},
    };

    for (auto& [cat, cmds] : groups) {
        std::cout << "\n=== " << titles[cat] << " ===\n";
        for (auto& [name, desc] : cmds)
            printf("  %-18s - %s\n", name.c_str(), desc.c_str());
    }
    printf("\n  %-18s - %s\n", "exit", "Exit the simulator");
}

static struct _Init_Cmd {
    _Init_Cmd() {
        // System
        Cmd["help"]  = {cmd_help,  "Show this help message",          "System"};
        Cmd["clear"] = {cmd_clear, "Clear terminal screen",          "System"};
        // Process
        Cmd["create_pcb"] = {cmd_create_pcb, "Create a new process",         "Process"};
        Cmd["show_pcb"]   = {cmd_show_pcb,   "Show process details",         "Process"};
        Cmd["list_pcb"]   = {cmd_list_pcb,   "List all processes",           "Process"};
        Cmd["renice"]     = {cmd_renice,     "Change process priority",      "Process"};
        Cmd["block_pcb"]  = {cmd_block_pcb,  "Block a process",              "Process"};
        Cmd["wakeup_pcb"] = {cmd_wakeup_pcb, "Wake up a blocked process",    "Process"};
        Cmd["suspend"]    = {cmd_suspend_pcb,"Suspend a process",            "Process"};
        Cmd["resume"]     = {cmd_resume_pcb, "Resume a suspended process",   "Process"};
        Cmd["ptree"]      = {cmd_ptree,      "Show process tree",            "Process"};
        Cmd["kill_pcb"]   = {cmd_kill_pcb,   "Kill a process",               "Process"};
        // Memory
        Cmd["set_alloc_algo"] = {cmd_set_alloc_algo, "Set allocation algorithm", "Memory"};
        Cmd["show_mem"]       = {cmd_show_mem,       "Show memory map",          "Memory"};
        Cmd["mem_stat"]       = {cmd_mem_stat,       "Show memory statistics",   "Memory"};
        Cmd["alloc"]          = {cmd_alloc,          "Allocate memory to process","Memory"};
        Cmd["free_mem"]       = {cmd_free_mem,       "Free process memory",       "Memory"};
        Cmd["compact"]        = {cmd_compact,        "Compact memory",             "Memory"};
        Cmd["pgfault"]        = {cmd_pgfault,        "Simulate a page fault",      "Memory"};
        Cmd["swap_out"]       = {cmd_swap_out,       "Swap out process memory",    "Memory"};
    }
} _init_Cmd;

void dispatch(const std::vector<std::string>& args) {
    if (args.empty()) return;
    auto it = Cmd.find(args[0]);
    if (it != Cmd.end()) {
        it->second.handler(args);
    } else {
        std::cout << "Unknown command: " << args[0] << '\n';
    }
}