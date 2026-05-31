#pragma once

#include "common.h"
#include <list>

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

void cmd_set_alloc_algo(const std::vector<std::string>&);

void cmd_show_mem(const std::vector<std::string>&);

void cmd_mem_stat(const std::vector<std::string>&);

void cmd_alloc(const std::vector<std::string>&);

void cmd_free_mem(const std::vector<std::string>&);

void cmd_compact(const std::vector<std::string>&);
