#include <iostream>
#include <iomanip>
#include <map>
#include "process.h"
#include "common.h"

static std::map<int, PCB> pcb_table;
static int cur_pid = -1;

int get_tol_size(const std::vector<Proc_Mem_Blo>& _mem) {
    int tmp = 0;
    for (auto [_base, _size, _] : _mem) tmp += _size;
    return tmp;
}

static const char* state_name(Proc_State s) {
    switch (s) {
        case Proc_State::READY:     return "READY";
        case Proc_State::RUNNING:   return "RUNNING";
        case Proc_State::BLOCKED:   return "BLOCKED";
        case Proc_State::SUSPENDED: return "SUSPENDED";
        case Proc_State::ZOMBIE:    return "ZOMBIE";
        default:                    return "UNKNOWN";
    }
}

void init_processes() {
    cur_pid++;

    pcb_table[cur_pid].pid = cur_pid;
    pcb_table[cur_pid].ppid = -1;
    pcb_table[cur_pid].name = "init";
    pcb_table[cur_pid].state = Proc_State::RUNNING;
    pcb_table[cur_pid].priority = 0;

    pcb_table[cur_pid].cpu_time = 0;

    pcb_table[cur_pid].child.clear();
}

PCB* find_pcb(int pid) {
    auto it = pcb_table.find(pid);
    if (it != pcb_table.end()) return &(it->second);
    return nullptr;
}

void cmd_create_pcb(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "Usage: create_pcb <name> <priority>\n";
        return;
    }
    if (args[1].length() > MAX_PCB_NAME) {
        std::cout << "Error: name length cannot exceed " << MAX_PCB_NAME << '\n';
        return;
    }

    int prio = std::stoi(args[2]);

    if (prio > 15 || prio < 0) {
        std::cout << "Error: priority must be 0-15\n";
        return;        
    }

    int tried = 0;
    while (find_pcb(cur_pid)) {
        cur_pid++;
        if (cur_pid >= MAX_PID) cur_pid = 0;
        if (++tried >= MAX_PID) {
            std::cout << "Error: PID space exhausted\n";
            return;
        }
    }

    pcb_table[cur_pid].pid = cur_pid;
    pcb_table[cur_pid].ppid = 0;
    pcb_table[cur_pid].name = args[1];
    pcb_table[cur_pid].state = Proc_State::READY;
    pcb_table[cur_pid].priority = prio;

    pcb_table[cur_pid].mem.clear();
    pcb_table[cur_pid].cpu_time = 0;
    pcb_table[cur_pid].child.clear();

    pcb_table[0].child.emplace_back(cur_pid);

    std::cout << "[OK] Process created: pid=" << cur_pid 
    << ", name=" << args[1] << ", priority=" << prio << "\n";
}

void cmd_show_pcb(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: show_pcb <pid>\n";
        return;        
    }

    int pid = stoi(args[1]);

    if (pid < 0 || pid >= MAX_PID) {
        std::cout << "Error: pid invalid\n";
        return;
    }

    PCB* cur = find_pcb(pid);
    if (!cur) {
        std::cout << "Error: process " << pid << " not found\n";
        return;
    }

    if (args.size() == 2 || (args.size() >= 3 && args[2] == "all")) {
        const std::vector<std::string> buf = {"pid", "ppid", "name", "state", "priority",
        "mem", "cpu", "child"};
        for (std::string tar: buf) std::cout << cur->get_value(tar) << '\n';
    } else {
        std::cout << cur->get_value(args[2]) << '\n';
    }
}

void cmd_list_pcb(const std::vector<std::string>&) {
    if (pcb_table.empty()) {
        std::cout << "No processes.\n";
        return;
    }
    std::cout << std::left
              << std::setw(6)  << "PID"
              << std::setw(12) << "NAME"
              << std::setw(10) << "STATE"
              << std::setw(6)  << "PRIO"
              << std::setw(8)  << "MEM"
              << "CPU\n"
              << std::string(48, '-') << '\n';
    for (const auto& [_, p] : pcb_table) {
        std::cout << std::left
                  << std::setw(6)  << p.pid
                  << std::setw(12) << p.name
                  << std::setw(10) << state_name(p.state)
                  << std::setw(6)  << p.priority
                  << std::setw(8)  << (std::to_string(get_tol_size(p.mem)) + "KB")
                  << p.cpu_time << '\n';
    }
}

void cmd_renice(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "Usage: renice <pid> <new_priority>\n";
        return;
    }
    
    int pid = std::stoi(args[1]);

    if (pid < 0 || pid >= MAX_PID) {
        std::cout << "Error: pid invalid\n";
        return;
    }

    int prio = std::stoi(args[2]);

    if (prio > 15 || prio < 0) {
        std::cout << "Error: priority must be 0-15\n";
        return;        
    }
    
    PCB* cur = find_pcb(pid);
    if (!cur) {
        std::cout << "Error: process " << pid << " not found\n";
        return;
    }
    int _old = cur->priority;
    cur->priority = prio;
    std::cout << "[OK] Change process " << pid << " priority "
              << _old << " to " << prio << '\n';
}

void cmd_block_pcb(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: block_pcb <pid>\n";
        return;
    }

    int pid = std::stoi(args[1]);

    if (pid < 0 || pid >= MAX_PID) {
        std::cout << "Error: pid invalid\n";
        return;
    }

    PCB* cur = find_pcb(pid);
    if (!cur) {
        std::cout << "Error: process " << pid << " not found\n";
        return;
    }
    if (cur->state == Proc_State::BLOCKED) {
        std::cout << "Process " << pid <<  " is already blocked\n";
        return;
    }
    cur->state = Proc_State::BLOCKED;
    std::cout << "[OK] Block process " << pid << '\n';
}

void cmd_wakeup_pcb(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: wakeup_pcb <pid>\n";
        return;
    }

    int pid = std::stoi(args[1]);

    if (pid < 0 || pid >= MAX_PID) {
        std::cout << "Error: pid invalid\n";
        return;
    }

    PCB* cur = find_pcb(pid);
    if (!cur) {
        std::cout << "Error: process " << pid << " not found\n";
        return;
    }
    if (cur->state != Proc_State::BLOCKED) {
        std::cout << "Process " << pid << " is not blocked\n";
        return;
    }
    cur->state = Proc_State::READY;
    std::cout << "[OK] Wakeup process " << pid << '\n';
}

void cmd_suspend_pcb(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: suspend_pcb <pid>\n";
        return;
    }

    int pid = std::stoi(args[1]);

    if (pid < 0 || pid >= MAX_PID) {
        std::cout << "Error: pid invalid\n";
        return;
    }

    PCB* cur = find_pcb(pid);
    if (!cur) {
        std::cout << "Error: process " << pid << " not found\n";
        return;
    }
    if (cur->state == Proc_State::SUSPENDED) {
        std::cout << "Process " << pid << " already suspended\n";
        return;
    }
    cur->state = Proc_State::SUSPENDED;
    std::cout << "[OK] Suspend process " << pid << '\n';
}

void cmd_resume_pcb(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: resume_pcb <pid>\n";
        return;
    }

    int pid = std::stoi(args[1]);

    if (pid < 0 || pid >= MAX_PID) {
        std::cout << "Error: pid invalid\n";
        return;
    }

    PCB* cur = find_pcb(pid);
    if (!cur) {
        std::cout << "Error: process " << pid << " not found\n";
        return;
    }
    if (cur->state != Proc_State::SUSPENDED) {
        std::cout << "Process " << pid << " is not suspended\n";
        return;
    }
    cur->state = Proc_State::READY;
    std::cout << "[OK] Resume process " << pid << '\n';
}

static std::string ptree_node(PCB* p) {
    return p->name + "(" + std::to_string(p->pid) + ") [" +
           state_name(p->state) + ", prio=" + std::to_string(p->priority) +
           ", mem=" + std::to_string(get_tol_size(p->mem)) + "KB]";
}

static void print_tree(int pid, const std::string& prefix, bool is_last) {
    PCB* cur = find_pcb(pid);
    if (!cur) return;

    std::cout << prefix;
    if (pid != 0)
        std::cout << (is_last ? "└─ " : "├─ ");
    std::cout << ptree_node(cur) << '\n';

    for (size_t i = 0; i < cur->child.size(); i++)
        print_tree(cur->child[i],
                   prefix + (pid != 0 ? (is_last ? "   " : "│  ") : ""),
                   i == cur->child.size() - 1);
}

void cmd_ptree(const std::vector<std::string>&) {
    if (!find_pcb(0)) {
        std::cout << "No processes.\n";
    } else {
        print_tree(0, "", false);
    }
}

void run_kill(int pid) {
    PCB* cur = find_pcb(pid);
    if (!cur) return;

    for (int cpid : cur->child) run_kill(cpid);

    PCB* parent = find_pcb(cur->ppid);
    if (parent) {
        auto& f = parent->child;
        std::erase(f, pid);
    }

    pcb_table.erase(pid);
}

void cmd_kill_pcb(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: kill_pcb <pid>\n";
        return;
    }

    int pid = std::stoi(args[1]);

    if (pid <= 0 || pid >= MAX_PID) {
        std::cout << "Error: pid invalid\n";
        return;        
    }

    PCB* cur = find_pcb(pid);

    if (!cur) std::cout << "No process.\n";
    else {
        run_kill(pid);
        std::cout << "[OK] Killed process " << pid << '\n';
    }
}