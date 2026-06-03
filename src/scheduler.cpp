#include <iostream>
#include <algorithm>
#include <sstream>
#include <chrono>
#include "scheduler.h"
#include "process.h"
#include "commands.h"

static int pid_to_queue(int priority) {
    for (int q = 0; q < Q_COUNT; q++) {
        if (priority >= Q_RANGES[q][0] && priority <= Q_RANGES[q][1]) return q;
    }
    return Q_COUNT - 1;
}

static std::thread sched_thread;

void init_scheduler() {
    for (int q = 0; q < Q_COUNT; q++) queues[q].clear();
    ticks_left = 0;
    total_ticks = 0;
    running_pid = 0;
}

void sched_enqueue(int pid) {
    if (pid < 3 || pid >= MAX_PID) {
        std::cout << "Error: pid invalid\n";
        return;
    }

    PCB* p = find_pcb(pid);

    if (!p) {
        std::cout << "Error: process " << pid << " not found\n";
        return;
    }

    if (p->state == Proc_State::BLOCKED || p->state == Proc_State::SUSPENDED) {
        std::cout << "Error: process is blocked or suspended\n";
        return;
    }

    int q = pid_to_queue(p->priority);

    queues[q].emplace_back(pid);

    p->current_queue = q;
}

void sched_dequeue(int pid) {
    if (pid < 3 || pid >= MAX_PID) {
        std::cout << "Error: pid invalid\n";
        return;
    }

    PCB* p = find_pcb(pid);

    if (!p) {
        std::cout << "Error: process " << pid << " not found\n";
        return;
    }

    if (p->state == Proc_State::RUNNING) {
        std::cout << "Error: cannot dequeue running process\n";
        return;
    }

    for (int q = 0; q < Q_COUNT; q++) {
        auto& dq = queues[q];

        auto it = std::find(dq.begin(), dq.end(), pid);

        if (it != dq.end()) {
            dq.erase(it);
            return;
        }
    }
}

static void sched_pick_next() {
    for (int q = 0; q < Q_COUNT; q++) {
        if (!queues[q].empty()) {
            PCB* idle = find_pcb(0);
            if (idle) idle->state = Proc_State::READY;

            running_pid = queues[q].front();
            queues[q].pop_front();
            ticks_left = TIME_SLICES[q];

            PCB* p = find_pcb(running_pid);
            if (p) p->state = Proc_State::RUNNING;
            return;
        }
    }

    running_pid = 0;
    PCB* idle = find_pcb(0);
    if (idle) idle->state = Proc_State::RUNNING;
}

static std::vector<std::string> output;

void scheduler_tick() {
    total_ticks++;

    if (running_pid == 0) {
        sched_pick_next();
        return;
    }

    PCB* p = find_pcb(running_pid);

    if (!p) {
        running_pid = 0;
        sched_pick_next();
        return;
    }

    ticks_left--;
    p->cpu_time++;

    if (p->cpu_time >= p->cpu_needed) {
        std::string km = "[OK] Process " + p->name + "("
                       + std::to_string(p->pid) + ") completed\n";
        output.emplace_back(km);
        run_kill(p->pid);
        sched_pick_next();
    } else if (!ticks_left) {
        int q = p->current_queue;
        int next_q = std::min(q + 1, Q_COUNT - 1);

        p->state = Proc_State::READY;
        p->current_queue = next_q;

        queues[next_q].emplace_back(running_pid);
        sched_pick_next();
    }
}

void cmd_step(const std::vector<std::string>&) {
    int prev_pid = running_pid;
    std::string prev_name;
    int prev_cpu = 0, prev_needed = 0;
    if (PCB* prev = find_pcb(prev_pid)) {
        prev_name = prev->name;
        prev_cpu = prev->cpu_time;
        prev_needed = prev->cpu_needed;
    }

    scheduler_tick();

    if (prev_pid != 0) {
        PCB* p = find_pcb(prev_pid);
        if (p) {
            std::cout << "[STEP] pid=" << prev_pid
                    << " name=" << p->name
                    << " cpu=" << p->cpu_time << "/" << p->cpu_needed;
            if (p->state != Proc_State::RUNNING)
                std::cout << " (time slice expired)";
            std::cout << "\n";
        } else {
            std::cout << "[STEP] pid=" << prev_pid
                    << " name=" << prev_name
                    << " cpu=" << (prev_cpu + 1) << "/" << prev_needed
                    << " [completed]\n";
        }
    } else {
        std::cout << "[STEP] idle\n";
    }
}

void start_background() {
    std::thread([]() {
        while (true) {
            SchedMsg* msg = nullptr;

            {
                std::lock_guard<std::mutex> lock(msg_mtx);
                if (!msg_queue.empty()) {
                    msg = msg_queue.front();
                    msg_queue.pop();
                }
            }

            if (msg) {
                std::stringstream ss;
                auto old_buf = std::cout.rdbuf(ss.rdbuf());
                {
                    std::lock_guard<std::mutex> lock(sched_mtx);
                    dispatch_direct(msg->args);
                }
                std::cout.rdbuf(old_buf);

                msg->result = ss.str();
                {
                    std::lock_guard<std::mutex> lock(msg_mtx);
                    msg->done = true;
                }
                done_cv.notify_all();
            }

            std::unique_lock<std::mutex> lock(msg_mtx);
            msg_cv.wait_for(lock, std::chrono::milliseconds(200),
                            [] { return !msg_queue.empty(); });
        }
    }).detach();
}

void sched_loop() {
    while (sched_running) {
        std::stringstream ss;
        auto old_buf = std::cout.rdbuf(ss.rdbuf());
        
        {
            std::lock_guard<std::mutex> lock(sched_mtx);
            cmd_step({});
        }
        
        std::cout.rdbuf(old_buf);

        output.emplace_back(ss.str());

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void cmd_start(const std::vector<std::string>&) {
    if (sched_running) {
        std::cout << "[INFO] Scheduler is already running\n";
        return;
    }
    output.clear();
    sched_running = true;

    sched_thread = std::thread(sched_loop);

    std::cout << "[OK] Scheduler started\n";
}

void cmd_stop(const std::vector<std::string>&) {
    if (!sched_running) {
        std::cout << "[INFO] Scheduler is not running\n";
        return;
    }
    sched_running = false;
    if (sched_thread.joinable()) sched_thread.join();

    for (const std::string& info : output)
        std::cout << info;
    output.clear();

    std::cout << "[OK] Scheduler stopped\n";
}

void cmd_restart(const std::vector<std::string>&) {
    cmd_stop({});
    cmd_start({});
}

void save_scheduler(std::ofstream& f) {
    f.write(reinterpret_cast<const char*>(&total_ticks), sizeof(total_ticks));
    f.write(reinterpret_cast<const char*>(&running_pid), sizeof(running_pid));
    f.write(reinterpret_cast<const char*>(&ticks_left),  sizeof(ticks_left));
    for (int q = 0; q < Q_COUNT; q++) {
        int sz = static_cast<int>(queues[q].size());
        f.write(reinterpret_cast<const char*>(&sz), sizeof(sz));
        for (int pid : queues[q])
            f.write(reinterpret_cast<const char*>(&pid), sizeof(pid));
    }
}

void load_scheduler(std::ifstream& f) {
    f.read(reinterpret_cast<char*>(&total_ticks), sizeof(total_ticks));
    f.read(reinterpret_cast<char*>(&running_pid), sizeof(running_pid));
    f.read(reinterpret_cast<char*>(&ticks_left),  sizeof(ticks_left));
    for (int q = 0; q < Q_COUNT; q++) {
        queues[q].clear();
        int sz = 0;
        f.read(reinterpret_cast<char*>(&sz), sizeof(sz));
        for (int i = 0; i < sz; i++) {
            int pid;
            f.read(reinterpret_cast<char*>(&pid), sizeof(pid));
            queues[q].push_back(pid);
        }
    }
}
