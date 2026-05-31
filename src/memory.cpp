#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <list>
#include <algorithm>
#include <unordered_map>
#include <functional>
#include <set>
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

void Merge_Fre_Mem_Blo() {
    for (auto it = Mem.begin(); it != Mem.end(); it++) {
        auto ne = it;
        ne++;

        while (ne != Mem.end() && ne->is_free) {
            it->size += ne->size;
            Mem.erase(ne);
            ne = it;
            ne++;
        }
    }
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
    int tol_fr = 0;
    int tol_al = 0;
    int fr_blo = 0;
    int max_fr_blo = 0;
    for (auto& it : Mem) {
        if (it.is_free) {
            max_fr_blo = std::max(max_fr_blo, it.size);
            tol_fr += it.size;
            fr_blo++;
        } else {
            tol_al += it.size;
        }
    }

    double frag_rate = (tol_fr > 0) ? (1.0 - 1.0 * max_fr_blo / tol_fr) : 0.0;

    std::cout << "=== Memory Statistics ===\n"
              << "Total:    " << TOTAL_MEM_KB << " KB\n"
              << "Allocated:" << std::setw(5) << tol_al << " KB\n"
              << "Free:     " << std::setw(5) << tol_fr << " KB\n"
              << "Free blocks: " << fr_blo << "\n"
              << "Largest free: " << max_fr_blo << " KB\n"
              << "Fragmentation: " << std::fixed << std::setprecision(1)
              << frag_rate * 100 << "%\n";
}

MemBlock* find_mem_ff(int req) {
    for (auto& it : Mem) {
        if (it.is_free && it.size >= req) return &it;
    }
    return nullptr;
}

using mem_find = std::function<MemBlock*(int)>;

MemBlock* find_mem_bf(int req) {
    MemBlock* fit = nullptr;

    for (auto& it : Mem) {
        if (it.is_free && it.size >= req && (!fit || fit->size > it.size))
            fit = &it;
    }

    return fit;
}

MemBlock* find_mem_wf(int req) {
    MemBlock* fit = nullptr;

    for (auto& it : Mem) {
        if (it.is_free && it.size >= req && (!fit || fit->size < it.size))
            fit = &it;
    }

    return fit;
}

void cmd_alloc(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "Usage: alloc <pid> <size_kb>\n";
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

    int req = std::stoi(args[2]);

    if (req > TOTAL_MEM_KB) {
        std::cout << "Error: total memory " << TOTAL_MEM_KB << " kb\n";
        return;
    }

    std::unordered_map<std::string, mem_find> algo_dispatch = {
        {"FIRST_FIT", find_mem_ff},
        {"BEST_FIT", find_mem_bf},
        {"WORST_FIT", find_mem_wf}
    };

    MemBlock* fit_block = algo_dispatch[get_algo_status()](req);

    if (!fit_block) {
        std::cout << "Error: Not enough memory\n";
        return;
    }

    cur->mem.emplace_back(Proc_Mem_Blo{fit_block->base, req});

    auto it = std::find_if(Mem.begin(), Mem.end(),
        [&](MemBlock& b) { return &b == fit_block; });

    Mem.emplace(it, MemBlock{fit_block->base, req, pid, false});

    fit_block->base += req;
    fit_block->size -= req;

    if (fit_block->size == 0)
        Mem.erase(it);

    std::cout << "[OK] Allocated " << req << "KB to process " << pid << '\n';
}

void cmd_free_mem(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: free_mem <pid>\n";
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

    std::sort(cur->mem.begin(), cur->mem.end(), 
    [](Proc_Mem_Blo& x, Proc_Mem_Blo& y) { return x.base < y.base; });

    int tol_fr = 0;

    for (auto tar : cur->mem) {
        auto it = Mem.begin();
        while (it->base != tar.base) it++;
        it->is_free = true;
        it->owner_pid = -1;
        tol_fr += it->size;
    }

    cur->mem.clear();

    std::cout << "[OK] Free " << std::to_string(tol_fr) << 
    "kb memory from process " << std::to_string(pid) << '\n';

    Merge_Fre_Mem_Blo();
}

void cmd_compact(const std::vector<std::string>&) {
    std::multiset<MemBlock> oc_bl;

    for (auto it : Mem) {
        if (!it.is_free) oc_bl.emplace(it);
    }

    int new_base = 0;

    if (!oc_bl.empty()) {
        Mem.clear();
        for (auto it : oc_bl) {
            int old_base = it.base;
            it.base = new_base;
            new_base += it.size;

            PCB* it_owner = find_pcb(it.owner_pid);

            auto owner_mem = 
            std::find_if(it_owner->mem.begin(), 
            it_owner->mem.end(), 
            [&](Proc_Mem_Blo& x) { return x.base == old_base; });

            owner_mem->base = it.base;

            Mem.emplace_back(it);
        }
    }

    int ne_base = Mem.back().base + Mem.back().size;


    if (ne_base < TOTAL_MEM_KB) {
        Mem.emplace_back(MemBlock{ne_base, 
                   TOTAL_MEM_KB - ne_base, -1, 
                true});
    }
    
    std::cout << "[OK] Compact\n";
}