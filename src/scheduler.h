#pragma once

#include <deque>
#include <thread>
#include <mutex>
#include <atomic>

constexpr int Q_COUNT = 3;
constexpr int Q_RANGES[3][2] = {{0, 3}, {4, 7}, {8, 15}};
constexpr int TIME_SLICES[3] = {1, 2, 4};

inline std::deque<int> queues[3];
inline int running_pid;
inline int ticks_left;
inline int total_ticks;
inline std::atomic<bool> sched_running{false};
inline std::thread sched_thread;
inline std::mutex sched_mtx;

void init_scheduler();

void sched_enqueue(int);

void sched_dequeue(int);

void cmd_step(const std::vector<std::string>&);

void cmd_start(const std::vector<std::string>&);

void cmd_stop(const std::vector<std::string>&);

void cmd_restart(const std::vector<std::string>&);