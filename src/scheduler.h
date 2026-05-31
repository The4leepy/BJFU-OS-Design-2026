#pragma once

#include <deque>

constexpr int Q_COUNT = 3;
constexpr int Q_RANGES[3][2] = {{0, 3}, {4, 7}, {8, 15}};
constexpr int TIME_SLICES[3] = {1, 2, 4};

extern std::deque<int> queues[3];
extern int current_pid;
extern int ticks_left;
extern int total_ticks;
extern bool running;

void init_scheduler();