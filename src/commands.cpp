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
    std::cout << "Available commands: \n";
    for (const auto& [name, info] : Cmd) {
        std::cout << name << " - "  << info.desc << '\n';
    }
    std::cout << "exit - Exit the simulator\n";
}

static struct _Init_Cmd {
    _Init_Cmd() {
        Cmd["help"] = {cmd_help, "Show this help message"};
        Cmd["clear"] = {cmd_clear, "Clear terminal"};
        Cmd["create_pcb"] = {cmd_create_pcb, "Create a new process"};
        Cmd["show_pcb"] = {cmd_show_pcb, "Show process pcb info"};
        Cmd["list_pcb"] = {cmd_list_pcb, "List all processes"};
        Cmd["renice"] = {cmd_renice, "Change process priority"};
        Cmd["block_pcb"] = {cmd_block_pcb, "Block process"};
        Cmd["wakeup_pcb"] = {cmd_wakeup_pcb, "Wakeup a blocked process"};
        Cmd["suspend"] = {cmd_suspend_pcb, "Suspend a process"};
        Cmd["resume"] = {cmd_resume_pcb, "Resume a suspended process"};
        Cmd["ptree"] = {cmd_ptree, "Show process tree"};
        Cmd["kill_pcb"]       = {cmd_kill_pcb, "Kill process"};
        Cmd["set_alloc_algo"] = {cmd_set_alloc_algo, "Set memory allocation algorithm"};
        Cmd["show_mem"]       = {cmd_show_mem, "Show memory map"};
        Cmd["mem_stat"]       = {cmd_mem_stat, "Show memory statistics"};
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