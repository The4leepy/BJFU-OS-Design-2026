#pragma once

#include <deque>

constexpr int Q_COUNT = 3;
constexpr int Q_RANGES[3][2] = {{0, 3}, {4, 7}, {8, 15}};
constexpr int TIME_SLICES[3] = {1, 2, 4};

inline std::deque<int> queues[3];
inline int current_pid;
inline int ticks_left;
inline int total_ticks;
inline bool running;

void init_scheduler();