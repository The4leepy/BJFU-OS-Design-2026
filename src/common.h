#pragma once

#include <vector>
#include <string>
#include <cstdint>

enum class Proc_State{
    READY,
    RUNNING,
    BLOCKED,
    SUSPENDED,
    ZOMBIE
};

enum class Mem_Alloc_Algo {
    FIRST_FIT,
    BEST_FIT,
    WORST_FIT
};

constexpr int TOTAL_MEM_KB = 1024;
constexpr int MAX_PCB_NAME = 32;
constexpr int MAX_PID = 32;