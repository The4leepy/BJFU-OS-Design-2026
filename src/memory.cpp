#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <list>
#include "memory.h"
#include "process.h"

static std::list<MemBlock> Mem;
static Mem_Alloc_Algo algo = Mem_Alloc_Algo::FIRST_FIT;

void init_memory() {
    Mem.emplace_back(MemBlock{0, 1024, -1, true});
}

std::string get_algo_status() {
    if (algo == Mem_Alloc_Algo::FIRST_FIT) return "FIRST_FIT";
    else if (algo == Mem_Alloc_Algo::BEST_FIT) return "BEST_FIT";
    else return "WORST_FIT";
}

void cmd_set_alloc_algo(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: set_alloc_algo <type>\n";
        return;
    }

    if (args[1] == "ff") {
        if (get_algo_status() == "FIRST_FIT") {
            std::cout << "Algorithm status is already FIRST_FIT\n";
            return;
        }
        algo = Mem_Alloc_Algo::FIRST_FIT;
    } else if (args[1] == "bf") {
        if (get_algo_status() == "BEST_FIT") {
            std::cout << "Algorithm status is already BEST_FIT\n";
            return;
        } 
        algo = Mem_Alloc_Algo::BEST_FIT;
    } else if (args[1] == "wf") {
        if (get_algo_status() == "WORST_FIT") {
            std::cout << "Algorithm status is already WORST_FIT\n";
            return;
        }
        algo = Mem_Alloc_Algo::WORST_FIT;
    } else {
        std::cout << "Error: unknown algorithm (use ff, bf, or wf)\n";
        return;
    }

    std::cout << "[OK] Allocation algorithm set to " << get_algo_status() << '\n';
}

void cmd_show_mem(const std::vector<std::string>&) {
    std::cout << std::left
              << std::setw(9) << "Start"
              << std::setw(9) << "Size"
              << std::setw(9) << "Owner"
              << std::setw(9) << "Status\n";
    for (auto& _mem : Mem) {
        std::string owner_name = "-";
        if (_mem.owner_pid >= 0) {
            PCB* owner = find_pcb(_mem.owner_pid);
            if (owner) owner_name = owner->name + "(" + std::to_string(_mem.owner_pid) + ")";
        }
        std::cout << std::left
              << std::setw(9) << _mem.base
              << std::setw(9) << _mem.size
              << std::setw(9) << owner_name
              << std::setw(9) << (_mem.is_free ? "Free" : "Alloc")
              << '\n';
    }

    // 可视化横条
    std::cout << "\nMemory Map (0-" << TOTAL_MEM_KB << "KB):\n";
    const int W = 64;
    for (auto& b : Mem) {
        int w = std::max(1, b.size * W / TOTAL_MEM_KB);
        if (b.is_free) {
            std::cout << std::string(w, '-');
        } else {
            std::cout << "|" << b.size << "KB"
                      << std::string(std::max(0, w - 4), '#');
        }
    }
    std::cout << "|\n";
}

void cmd_mem_stat(const std::vector<std::string>&) {
    // TODO: 待实现
    std::cout << "mem_stat: not yet implemented\n";
}