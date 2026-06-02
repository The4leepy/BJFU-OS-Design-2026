#include <iostream>
#include <algorithm>
#include "scheduler.h"
#include "process.h"

static int pid_to_queue(int priority) {
    for (int q = 0; q < Q_COUNT; q++) {
        if (priority >= Q_RANGES[q][0] && priority <= Q_RANGES[q][1]) return q;
    }
    return Q_COUNT - 1;
}

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

    if (!ticks_left) {
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
    scheduler_tick();
    if (prev_pid != 0) {
        PCB* p = find_pcb(prev_pid);
        if (p) {
            std::cout << "[STEP] pid=" << prev_pid
                    << " name=" << p->name
                    << " cpu=" << p->cpu_time;
            if (p->state != Proc_State::RUNNING)
                std::cout << " (time slice expired)";
            std::cout << "\n";
        }
    } else {
        std::cout << "[STEP] idle\n";
    }
}