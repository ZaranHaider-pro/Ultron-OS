#pragma once
#include "ultron.h"
#include "logger.h"

// =======================================================
//  SCHEDULER
//  Task 5:  CPU Scheduling – FCFS
//  Task 6:  CPU Scheduling – Round Robin
//  EXTENDED: Multi-Level Queue Scheduler
//
//   Level HIGH   (0) – Round Robin   (quantum = 3 ticks)
//   Level MEDIUM (1) – FCFS
//   Level LOW    (2) – Priority scheduling with Aging
//                      (prevents starvation)
//
//  Thread-safe via std::mutex + std::condition_variable
// =======================================================

static constexpr int RR_QUANTUM  = 3;   // ticks per Round Robin slice
static constexpr int AGING_TICKS = 5;   // waits before priority boost

class Scheduler {
private:
    std::deque<PCB*>        highQ;    // HIGH   → Round Robin
    std::deque<PCB*>        medQ;     // MEDIUM → FCFS
    std::deque<PCB*>        lowQ;     // LOW    → Priority + Aging

    mutable std::mutex      qMtx;
    std::condition_variable qCV;
    Logger&                 log;

    // RR state
    PCB* rrCurrent   = nullptr;
    int  rrSliceLeft = 0;

    // Aging counter
    int  agingCounter = 0;

    // -- Aging: boost priority of long-waiting LOW queue procs --
    void applyAging() {
        agingCounter++;
        if (agingCounter < AGING_TICKS) return;
        agingCounter = 0;
        for (auto* p : lowQ) {
            p->waitingTime++;
            if (p->priority > 1) {
                p->priority--;
                log.info("SCHED", "Aging: PID " + std::to_string(p->pid) +
                         " priority boosted to " + std::to_string(p->priority));
            }
        }
        std::stable_sort(lowQ.begin(), lowQ.end(),
            [](const PCB* a, const PCB* b){ return a->priority < b->priority; });
    }

public:
    Scheduler(Logger& l) : log(l) {}

    // -- Enqueue a process into the right queue ----------
    void enqueue(PCB* p) {
        std::lock_guard<std::mutex> lk(qMtx);
        p->state = ProcessState::READY;
        switch (p->qLevel) {
            case QueueLevel::HIGH:   highQ.push_back(p); break;
            case QueueLevel::MEDIUM: medQ.push_back(p);  break;
            case QueueLevel::LOW:    lowQ.push_back(p);  break;
        }
        log.info("SCHED", "PID " + std::to_string(p->pid) + " enqueued in " +
                 (p->qLevel==QueueLevel::HIGH ? "HIGH(RR)" :
                  p->qLevel==QueueLevel::MEDIUM ? "MED(FCFS)" : "LOW(Priority)") + " queue");
        qCV.notify_one();
    }

    // -- Dequeue next process to run ---------------------
    PCB* dequeue() {
        std::lock_guard<std::mutex> lk(qMtx);
        applyAging();

        // HIGH – Round Robin
        if (!highQ.empty()) {
            if (rrCurrent == nullptr || rrSliceLeft <= 0) {
                if (rrCurrent && rrCurrent->state == ProcessState::RUNNING) {
                    if (rrCurrent->remainingTime > 0) {
                        rrCurrent->state = ProcessState::READY;
                        highQ.push_back(rrCurrent);
                    }
                }
                rrCurrent   = highQ.front();
                highQ.pop_front();
                rrSliceLeft = RR_QUANTUM;
            }
            rrSliceLeft--;
            rrCurrent->state = ProcessState::RUNNING;
            return rrCurrent;
        }

        // MEDIUM – FCFS
        if (!medQ.empty()) {
            PCB* p = medQ.front();
            medQ.pop_front();
            p->state = ProcessState::RUNNING;
            return p;
        }

        // LOW – Priority (sorted by priority field)
        if (!lowQ.empty()) {
            PCB* p = lowQ.front();
            lowQ.pop_front();
            p->state = ProcessState::RUNNING;
            return p;
        }
        return nullptr;
    }

    // -- Block a process (interrupt) ---------------------
    void block(PCB* p) {
        std::lock_guard<std::mutex> lk(qMtx);
        p->state = ProcessState::BLOCKED;
        log.info("SCHED", "PID " + std::to_string(p->pid) + " BLOCKED");
    }

    // -- Resume a blocked/minimized process --------------
    void resume(PCB* p) {
        p->state = ProcessState::READY;
        enqueue(p);
        log.info("SCHED", "PID " + std::to_string(p->pid) + " RESUMED");
    }

    // -- Remove from all queues (on kill/terminate) -------
    void remove(int pid) {
        std::lock_guard<std::mutex> lk(qMtx);
        auto rm = [&](std::deque<PCB*>& q) {
            q.erase(std::remove_if(q.begin(), q.end(),
                [pid](PCB* p){ return p->pid == pid; }), q.end());
        };
        rm(highQ); rm(medQ); rm(lowQ);
        if (rrCurrent && rrCurrent->pid == pid) rrCurrent = nullptr;
    }

    // -- Print queue stats --------------------------------
    void printQueues() const {
        std::lock_guard<std::mutex> lk(qMtx);
        auto printQ = [](const std::deque<PCB*>& q, const std::string& label){
            std::cout << CYAN << "  " << label << ": " << RESET;
            if (q.empty()) { std::cout << "(empty)\n"; return; }
            for (const auto* p : q)
                std::cout << "[" << p->pid << ":" << p->name << "] ";
            std::cout << "\n";
        };
        std::cout << BOLD << "\n  Multi-Level Ready Queues:\n" << RESET;
        printQ(highQ, "HIGH  (RR)      ");
        printQ(medQ,  "MEDIUM(FCFS)    ");
        printQ(lowQ,  "LOW   (Priority)");
    }

    bool isEmpty() const {
        std::lock_guard<std::mutex> lk(qMtx);
        return highQ.empty() && medQ.empty() && lowQ.empty();
    }

    int totalWaiting() const {
        std::lock_guard<std::mutex> lk(qMtx);
        return (int)(highQ.size() + medQ.size() + lowQ.size());
    }
};
