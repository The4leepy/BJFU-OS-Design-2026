#pragma once

#include <deque>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <memory>
#include <fstream>
#include <vector>
#include <string>

constexpr int Q_COUNT = 3;
constexpr int Q_RANGES[3][2] = {{0, 3}, {4, 7}, {8, 15}};
constexpr int TIME_SLICES[3] = {1, 2, 4};

inline std::deque<int> queues[3];
inline int running_pid;
inline int ticks_left;
inline int total_ticks;

inline std::atomic<bool> sched_running{false};

struct SchedMsg {
    std::vector<std::string> args;
    std::string result;
    bool done = false;
};
inline std::queue<std::shared_ptr<SchedMsg>> msg_queue;
inline std::mutex msg_mtx;
inline std::mutex sched_mtx;
inline std::condition_variable msg_cv;
inline std::condition_variable done_cv;

inline bool is_master = true;

void init_scheduler();
void start_background();

void sched_enqueue(int);
void sched_dequeue(int);

void cmd_step(const std::vector<std::string>&);
void cmd_start(const std::vector<std::string>&);
void cmd_stop(const std::vector<std::string>&);
void cmd_restart(const std::vector<std::string>&);

void save_scheduler(std::ofstream&);
void load_scheduler(std::ifstream&);
