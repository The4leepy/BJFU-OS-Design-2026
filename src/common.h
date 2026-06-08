#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <optional>

enum class Proc_State{
    READY,
    RUNNING,
    BLOCKED,
    SUSPENDED,
    ZOMBIE
};

constexpr int TOTAL_MEM_KB = 1024;
constexpr int MAX_PCB_NAME = 32;
constexpr int MAX_PID = 32;

inline std::optional<int> parse_int(const std::string& s) {
    try {
        std::size_t pos;
        int v = std::stoi(s, &pos);
        if (pos != s.size()) return std::nullopt;
        return v;
    } catch (...) {
        return std::nullopt;
    }
}