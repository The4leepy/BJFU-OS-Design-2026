#pragma once

#include "common.h"
#include <list>
#include <fstream>

struct MemBlock {
    int base;
    int size;
    int owner_pid;
    bool is_free;

    bool operator<(const MemBlock& other) const {
        return base < other.base;
    }
};

void init_memory();

std::string get_algo_status();

void cmd_set_alloc_algo(const std::vector<std::string>&);

void cmd_show_mem(const std::vector<std::string>&);

void cmd_mem_stat(const std::vector<std::string>&);

void cmd_alloc(const std::vector<std::string>&);

void cmd_free_mem(const std::vector<std::string>&);

void cmd_compact(const std::vector<std::string>&);

void cmd_pgfault(const std::vector<std::string>&);

void cmd_swap_out(const std::vector<std::string>&);

void save_memory(std::ofstream&);

void load_memory(std::ifstream&);
