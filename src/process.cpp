#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <unordered_set>
#include "process.h"
#include "common.h"
#include "memory.h"
#include "user.h"
#include "scheduler.h"
#include "persistence.h"

static std::unordered_map<int, PCB> pcb_table;
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

static void create_process(int _ppid, int _priority, std::string name) {
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
    pcb_table[cur_pid].ppid = _ppid;
    pcb_table[cur_pid].name = name;
    pcb_table[cur_pid].state = Proc_State::READY;
    pcb_table[cur_pid].priority = _priority;
    pcb_table[cur_pid].current_queue = -1;

    pcb_table[cur_pid].owner_user = current_user;

    pcb_table[cur_pid].mem.clear();
    pcb_table[cur_pid].cpu_time = 0;
    pcb_table[cur_pid].child.clear();

    pcb_table[_ppid].child.emplace_back(cur_pid);

    if (cur_pid > 2) sched_enqueue(cur_pid);
}

void init_processes() {
    cur_pid++;

    pcb_table[cur_pid].pid = cur_pid;
    pcb_table[cur_pid].ppid = -1;
    pcb_table[cur_pid].name = "swapper";
    pcb_table[cur_pid].state = Proc_State::RUNNING;
    pcb_table[cur_pid].priority = 0;
    pcb_table[cur_pid].current_queue = -1;

    pcb_table[cur_pid].owner_user = "root";

    pcb_table[cur_pid].cpu_time = 0;

    pcb_table[cur_pid].child.clear();

    running_pid = 0;

    create_process(0, 0, "init");
    create_process(0, 0, "kthreadd");
}

PCB* find_pcb(int pid) {
    auto it = pcb_table.find(pid);
    if (it != pcb_table.end()) return &(it->second);
    return nullptr;
}

void cmd_create(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "Usage: create <name> <priority>\n";
        return;
    }
    if (args[1].length() > MAX_PCB_NAME) {
        std::cout << "Error: name length cannot exceed " << MAX_PCB_NAME << '\n';
        return;
    }

    auto _prio = parse_int(args[2]);
    if (!_prio) { std::cout << "Error: invalid number\n"; return; }
    int prio = *_prio;
    if (prio > 15 || prio < 0) {
        std::cout << "Error: priority must be 0-15\n";
        return;        
    }

    create_process(1, prio, args[1]);

    std::cout << "[OK] Process created: pid=" << cur_pid 
    << ", name=" << args[1] << ", priority=" << prio << "\n";
}

void cmd_show(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: show <pid>\n";
        return;        
    }

    auto _pid = parse_int(args[1]);
    if (!_pid) { std::cout << "Error: invalid number\n"; return; }
    int pid = *_pid;
    if (pid < 0 || pid >= MAX_PID) {
        std::cout << "Error: pid invalid\n";
        return;
    }

    PCB* p = find_pcb(pid);
    if (!p) {
        std::cout << "Error: process " << pid << " not found\n";
        return;
    }
    if (!can_access(p)) {
        std::cout << "Error: permission denied\n";
        return;
    }

    if (args.size() == 2 || (args.size() >= 3 && args[2] == "all")) {
        const std::vector<std::string> buf = {"pid", "ppid", "name", "state", "priority",
        "mem", "cpu", "child"};
        for (std::string tar: buf) std::cout << p->get_value(tar) << '\n';
    } else {
        std::cout << p->get_value(args[2]) << '\n';
    }
}

void cmd_list(const std::vector<std::string>&) {
    if (pcb_table.empty()) {
        std::cout << "No processes\n";
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
        if (!can_access(&p)) continue;
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
    
    auto _pid = parse_int(args[1]);
    if (!_pid) { std::cout << "Error: invalid number\n"; return; }
    int pid = *_pid;
    if (pid < 3 || pid >= MAX_PID) {
        std::cout << "Error: pid invalid\n";
        return;
    }

    auto _prio = parse_int(args[2]);
    if (!_prio) { std::cout << "Error: invalid number\n"; return; }
    int prio = *_prio;
    if (prio > 15 || prio < 0) {
        std::cout << "Error: priority must be 0-15\n";
        return;        
    }
    
    PCB* p = find_pcb(pid);
    if (!p) {
        std::cout << "Error: process " << pid << " not found\n";
        return;
    }
    if (!can_access(p)) {
        std::cout << "Error: permission denied\n";
        return;
    }

    int _old = p->priority;
    p->priority = prio;

    std::cout << "[OK] Change process " << pid << " priority "
              << _old << " to " << prio << '\n';
}

void cmd_block(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: block <pid>\n";
        return;
    }

    auto _pid = parse_int(args[1]);
    if (!_pid) { std::cout << "Error: invalid number\n"; return; }
    int pid = *_pid;
    if (pid < 3 || pid >= MAX_PID) {
        std::cout << "Error: pid invalid\n";
        return;
    }

    PCB* p = find_pcb(pid);
    if (!p) {
        std::cout << "Error: process " << pid << " not found\n";
        return;
    }
    if (!can_access(p)) {
        std::cout << "Error: permission denied\n";
        return;
    }
    if (p->state == Proc_State::BLOCKED) {
        std::cout << "[INFO] Process " << pid <<  " is already blocked\n";
        return;
    }

    p->state = Proc_State::BLOCKED;
    sched_dequeue(p->pid);
    
    std::cout << "[OK] Block process " << pid << '\n';
}

void cmd_wakeup(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: wakeup <pid>\n";
        return;
    }

    auto _pid = parse_int(args[1]);
    if (!_pid) { std::cout << "Error: invalid number\n"; return; }
    int pid = *_pid;
    if (pid < 3 || pid >= MAX_PID) {
        std::cout << "Error: pid invalid\n";
        return;
    }

    PCB* p = find_pcb(pid);
    if (!p) {
        std::cout << "Error: process " << pid << " not found\n";
        return;
    }
    if (!can_access(p)) {
        std::cout << "Error: permission denied\n";
        return;
    }
    if (p->state != Proc_State::BLOCKED) {
        std::cout << "[INFO] Process " << pid << " is not blocked\n";
        return;
    }

    p->state = Proc_State::READY;
    sched_enqueue(p->pid);

    std::cout << "[OK] Wakeup process " << pid << '\n';
}

void cmd_suspend(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: suspend <pid>\n";
        return;
    }

    auto _pid = parse_int(args[1]);
    if (!_pid) { std::cout << "Error: invalid number\n"; return; }
    int pid = *_pid;
    if (pid < 3 || pid >= MAX_PID) {
        std::cout << "Error: pid invalid\n";
        return;
    }

    PCB* p = find_pcb(pid);
    if (!p) {
        std::cout << "Error: process " << pid << " not found\n";
        return;
    }
    if (!can_access(p)) {
        std::cout << "Error: permission denied\n";
        return;
    }
    if (p->state == Proc_State::SUSPENDED) {
        std::cout << "[INFO] Process " << pid << " is already suspended\n";
        return;
    }

    p->state = Proc_State::SUSPENDED;
    sched_dequeue(p->pid);

    std::cout << "[OK] Suspend process " << pid << '\n';
}

void cmd_resume(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: resume <pid>\n";
        return;
    }

    auto _pid = parse_int(args[1]);
    if (!_pid) { std::cout << "Error: invalid number\n"; return; }
    int pid = *_pid;
    if (pid < 3 || pid >= MAX_PID) {
        std::cout << "Error: pid invalid\n";
        return;
    }

    PCB* p = find_pcb(pid);
    if (!p) {
        std::cout << "Error: process " << pid << " not found\n";
        return;
    }
    if (!can_access(p)) {
        std::cout << "Error: permission denied\n";
        return;
    }
    if (p->state != Proc_State::SUSPENDED) {
        std::cout << "[INFO] Process " << pid << " is not suspended\n";
        return;
    }

    p->state = Proc_State::READY;
    sched_enqueue(p->pid);

    std::cout << "[OK] Resume process " << pid << '\n';
}

static std::string ptree_node(PCB* p) {
    return p->name + "(" + std::to_string(p->pid) + ") [" +
           state_name(p->state) + ", prio=" + std::to_string(p->priority) +
           ", mem=" + std::to_string(get_tol_size(p->mem)) + "KB]";
}

static void print_tree(int pid, const std::string& prefix, bool is_last,
                       std::unordered_set<int>& visited) {
    if (!visited.insert(pid).second) return;
    
    PCB* p = find_pcb(pid);
    if (!p) return;
    if (!can_access(p)) return;

    std::cout << prefix;
    if (pid != 0)
        std::cout << (is_last ? "└─ " : "├─ ");
    std::cout << ptree_node(p) << '\n';

    for (size_t i = 0; i < p->child.size(); i++)
        print_tree(p->child[i],
                   prefix + (pid != 0 ? (is_last ? "   " : "│  ") : ""),
                   i == p->child.size() - 1, visited);
}

void cmd_ptree(const std::vector<std::string>&) {
    if (!find_pcb(0)) {
        std::cout << "No processes\n";
    } else {
        std::unordered_set<int> visited;
        print_tree(0, "", false, visited);
    }
}

static void kill_impl(int pid, std::unordered_set<int>& visited) {
    if (!visited.insert(pid).second) return;
    PCB* p = find_pcb(pid);
    if (!p) return;

    for (int cpid : p->child) kill_impl(cpid, visited);

    PCB* parent = find_pcb(p->ppid);
    if (parent) {
        auto& f = parent->child;
        std::erase(f, pid);
    }

    p->state = Proc_State::SUSPENDED;
    sched_dequeue(p->pid);

    pcb_table.erase(pid);

    cmd_free_mem({"free_mem",std::to_string(pid)});
}

void run_kill(int pid) {
    std::unordered_set<int> visited;
    kill_impl(pid, visited);
}

void cmd_kill(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: kill <pid>\n";
        return;
    }

    auto _pid = parse_int(args[1]);
    if (!_pid) { std::cout << "Error: invalid number\n"; return; }
    int pid = *_pid;
    if (pid <= 3 || pid >= MAX_PID) {
        std::cout << "Error: pid invalid\n";
        return;        
    }

    PCB* p = find_pcb(pid);

    if (!p) std::cout << "Error: process " << pid << " not found\n";
    else if (!can_access(p)) std::cout << "Error: permission denied\n";
    else {
        run_kill(pid);
        std::cout << "[OK] Killed process " << pid << '\n';
    }
}

void save_processes(std::ofstream& f) {
    int sz = static_cast<int>(pcb_table.size());
    f.write(reinterpret_cast<const char*>(&sz), sizeof(sz));
    
    for (const auto& [_, p] : pcb_table) {
        f.write(reinterpret_cast<const char*>(&p.pid),           sizeof(p.pid));
        f.write(reinterpret_cast<const char*>(&p.ppid),          sizeof(p.ppid));
        write_str(f, p.name);
        int st = static_cast<int>(p.state);
        f.write(reinterpret_cast<const char*>(&st),              sizeof(st));
        f.write(reinterpret_cast<const char*>(&p.priority),      sizeof(p.priority));
        f.write(reinterpret_cast<const char*>(&p.current_queue), sizeof(p.current_queue));
        write_str(f, p.owner_user);
        f.write(reinterpret_cast<const char*>(&p.cpu_time),      sizeof(p.cpu_time));

        int child_sz = static_cast<int>(p.child.size());
        f.write(reinterpret_cast<const char*>(&child_sz), sizeof(child_sz));
        for (int c : p.child)
            f.write(reinterpret_cast<const char*>(&c), sizeof(c));

        int mem_sz = static_cast<int>(p.mem.size());
        f.write(reinterpret_cast<const char*>(&mem_sz), sizeof(mem_sz));
        for (const auto& blk : p.mem) {
            f.write(reinterpret_cast<const char*>(&blk.base), sizeof(blk.base));
            f.write(reinterpret_cast<const char*>(&blk.size), sizeof(blk.size));
            char sw = blk.is_swaped ? 1 : 0;
            f.write(&sw, 1);
        }
    }
}

void load_processes(std::ifstream& f) {
    pcb_table.clear();
    cur_pid = -1;
    int sz = 0;
    f.read(reinterpret_cast<char*>(&sz), sizeof(sz));
    for (int i = 0; i < sz; i++) {
        PCB p{};
        f.read(reinterpret_cast<char*>(&p.pid),           sizeof(p.pid));
        f.read(reinterpret_cast<char*>(&p.ppid),          sizeof(p.ppid));
        p.name        = read_str(f);
        int st = 0;
        f.read(reinterpret_cast<char*>(&st),              sizeof(st));
        p.state       = static_cast<Proc_State>(st);
        f.read(reinterpret_cast<char*>(&p.priority),      sizeof(p.priority));
        f.read(reinterpret_cast<char*>(&p.current_queue), sizeof(p.current_queue));
        p.owner_user  = read_str(f);
        f.read(reinterpret_cast<char*>(&p.cpu_time),      sizeof(p.cpu_time));

        int child_sz = 0;
        f.read(reinterpret_cast<char*>(&child_sz), sizeof(child_sz));
        p.child.resize(child_sz);
        for (int j = 0; j < child_sz; j++)
            f.read(reinterpret_cast<char*>(&p.child[j]), sizeof(p.child[j]));

        int mem_sz = 0;
        f.read(reinterpret_cast<char*>(&mem_sz), sizeof(mem_sz));
        p.mem.resize(mem_sz);
        for (int j = 0; j < mem_sz; j++) {
            f.read(reinterpret_cast<char*>(&p.mem[j].base), sizeof(p.mem[j].base));
            f.read(reinterpret_cast<char*>(&p.mem[j].size), sizeof(p.mem[j].size));
            char sw;
            f.read(&sw, 1);
            p.mem[j].is_swaped = (sw != 0);
        }

        if (p.pid > cur_pid) cur_pid = p.pid;
        pcb_table[p.pid] = std::move(p);
    }
}