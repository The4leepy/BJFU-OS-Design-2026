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
#include "user.h"

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

        while (ne != Mem.end() && it->is_free && ne->is_free) {
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
            std::cout << "[INFO] Algorithm is already set to FIRST_FIT\n";
            return;
        }
        algo = Mem_Alloc_Algo::FIRST_FIT;
    } else if (args[1] == "bf") {
        if (get_algo_status() == "BEST_FIT") {
            std::cout << "[INFO] Algorithm is already set to BEST_FIT\n";
            return;
        } 
        algo = Mem_Alloc_Algo::BEST_FIT;
    } else if (args[1] == "wf") {
        if (get_algo_status() == "WORST_FIT") {
            std::cout << "[INFO] Algorithm is already set to WORST_FIT\n";
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
              << std::setw(7)  << "Start"
              << std::setw(7)  << "Size"
              << std::setw(22) << "Owner"
              << "Status\n"
              << std::string(44, '-') << '\n';
    for (auto& _mem : Mem) {
        std::string owner_name = "-";
        if (_mem.owner_pid >= 0) {
            PCB* p = find_pcb(_mem.owner_pid);
            if (p) owner_name = p->name + "(" + std::to_string(_mem.owner_pid) + ")";
        }
        std::cout << std::left
              << std::setw(7)  << _mem.base
              << std::setw(7)  << _mem.size
              << std::setw(22) << owner_name
              << (_mem.is_free ? "Free" : "Alloc")
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

    auto _req = parse_int(args[2]);

    if (!_req) { std::cout << "Error: invalid number\n"; return; }

    int req = *_req;
    if (req <= 0) {
        std::cout << "Error: size must be positive\n";
        return;
    }

    if (req > TOTAL_MEM_KB) {
        std::cout << "Error: request exceeds total memory (" << TOTAL_MEM_KB << " KB)\n";
        return;
    }

    std::unordered_map<std::string, mem_find> algo_dispatch = {
        {"FIRST_FIT", find_mem_ff},
        {"BEST_FIT", find_mem_bf},
        {"WORST_FIT", find_mem_wf}
    };

    MemBlock* fit_block = algo_dispatch[get_algo_status()](req);

    if (!fit_block) {
        std::cout << "Error: not enough memory\n";
        return;
    }

    p->mem.emplace_back(Proc_Mem_Blo{fit_block->base, req});

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

    std::sort(p->mem.begin(), p->mem.end(),
    [](Proc_Mem_Blo& x, Proc_Mem_Blo& y) { return x.base < y.base; });

    int tol_fr = 0;

    for (const auto& tar : p->mem) {
        auto it = Mem.begin();
        while (it != Mem.end() && it->base != tar.base) it++;
        if (it == Mem.end()) break;
        it->is_free = true;
        it->owner_pid = -1;
        tol_fr += it->size;
    }

    p->mem.clear();

    std::cout << "[OK] Free " << std::to_string(tol_fr) << 
    "KB memory from process " << std::to_string(pid) << '\n';

    Merge_Fre_Mem_Blo();
}

void free_process_mem(int pid) {
    PCB* p = find_pcb(pid);
    if (!p) return;

    std::sort(p->mem.begin(), p->mem.end(),
    [](Proc_Mem_Blo& x, Proc_Mem_Blo& y) { return x.base < y.base; });

    for (const auto& tar : p->mem) {
        auto it = Mem.begin();
        while (it != Mem.end() && it->base != tar.base) it++;
        if (it == Mem.end()) break;
        it->is_free = true;
        it->owner_pid = -1;
    }
    p->mem.clear();
    Merge_Fre_Mem_Blo();
}

void cmd_compact(const std::vector<std::string>&) {
    std::multiset<MemBlock> oc_bl;

    for (const auto& b : Mem) {
        if (!b.is_free) oc_bl.emplace(b);
    }

    int new_base = 0;

    if (!oc_bl.empty()) {
        Mem.clear();
        for (auto it : oc_bl) {
            int old_base = it.base;
            it.base = new_base;
            new_base += it.size;

            PCB* pp = find_pcb(it.owner_pid);

            if (pp) {
                auto owner_mem =
                std::find_if(pp->mem.begin(),
                pp->mem.end(),
                [&](Proc_Mem_Blo& x) { return x.base == old_base; });

                if (owner_mem != pp->mem.end()) owner_mem->base = it.base;
            } else it.owner_pid = -1;

            Mem.emplace_back(it);
        }
    }

    int ne_base = Mem.back().base + Mem.back().size;


    if (ne_base < TOTAL_MEM_KB) {
        Mem.emplace_back(MemBlock{ne_base, 
                   TOTAL_MEM_KB - ne_base, -1, 
                true});
    }
    
    std::cout << "[OK] Memory compacted\n";
}

void cmd_pgfault(const std::vector<std::string>&) {
    std::cout << "[INFO] Page fault triggered — page replacement would occur here\n";
}

void cmd_swap_out(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "Usage: swap_out <pid> <size_kb>\n";
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

    auto _tar_kb = parse_int(args[2]);
    if (!_tar_kb) { std::cout << "Error: invalid number\n"; return; }
    int tar_kb = *_tar_kb;
    std::sort(p->mem.begin(), p->mem.end(),
    [](Proc_Mem_Blo& x, Proc_Mem_Blo& y){ return x.size < y.size; } );

    int swaped_kb = 0;
    
    for (auto& it : p->mem) {
        if (swaped_kb >= tar_kb) break;

        auto it_mem = std::find_if(Mem.begin(), Mem.end(),
        [&](MemBlock& x) {return it.base == x.base;} );
        if (it_mem != Mem.end()) Mem.erase(it_mem);

        it.is_swaped = 1;
        it.base = -1;
        swaped_kb += it.size;
    }

    std::cout << "[INFO] Swapped out " << swaped_kb
    << "KB from process " << p->name << "(" << std::to_string(pid) << ")\n";
}

void print_mem_map() {
    const int W = 64;
    for (const auto& b : Mem) {
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

void save_memory(std::ofstream& f) {
    int sz = static_cast<int>(Mem.size());
    f.write(reinterpret_cast<const char*>(&sz), sizeof(sz));

    for (const auto& b : Mem) {
        f.write(reinterpret_cast<const char*>(&b.base),      sizeof(b.base));
        f.write(reinterpret_cast<const char*>(&b.size),      sizeof(b.size));
        f.write(reinterpret_cast<const char*>(&b.owner_pid), sizeof(b.owner_pid));
        char free_flag = b.is_free ? 1 : 0;
        f.write(&free_flag, 1);
    }

    int a = static_cast<int>(algo);
    f.write(reinterpret_cast<const char*>(&a), sizeof(a));
}

void load_memory(std::ifstream& f) {
    Mem.clear();

    int sz = 0;
    f.read(reinterpret_cast<char*>(&sz), sizeof(sz));

    for (int i = 0; i < sz; i++) {
        MemBlock b{};
        f.read(reinterpret_cast<char*>(&b.base),      sizeof(b.base));
        f.read(reinterpret_cast<char*>(&b.size),      sizeof(b.size));
        f.read(reinterpret_cast<char*>(&b.owner_pid), sizeof(b.owner_pid));
        char free_flag;
        f.read(&free_flag, 1);
        b.is_free = (free_flag != 0);
        Mem.push_back(b);
    }

    int a = 0;
    f.read(reinterpret_cast<char*>(&a), sizeof(a));
    
    algo = static_cast<Mem_Alloc_Algo>(a);
}