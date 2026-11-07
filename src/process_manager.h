#pragma once
#include "ultron.h"
#include "logger.h"
#include "scheduler.h"

// =======================================================
//  PROCESS MANAGER
//  Task 3:  Process Creation & Termination
//  Task 4:  PCB Implementation
//  Task 5:  CPU Scheduling – FCFS
//  Task 6:  CPU Scheduling – Round Robin
//  Task 9:  Deadlock Detection (resource-allocation graph)
//  Task 13: Inter-Process Communication (message passing)
//  Task 14: System Calls Simulation (fork/exit/wait/getpid)
//
//  EXTENDED (new features):
//  • Multi-Level Queue Scheduling (HIGH/MEDIUM/LOW)
//  • Real Windows child process launch via CreateProcess
//    (equivalent of fork+exec; simulated mode if .exe absent)
//  • Interrupt simulation: RUNNING → BLOCKED → READY
//  • Minimize / Restore (MINIMIZED state)
//  • User mode / Kernel mode toggle
//  • Context switch simulation with timing
// =======================================================
class ProcessManager {
private:
    std::vector<PCB>                       processList;
    mutable std::mutex                     tableMtx;
    std::atomic<int>                       nextPid{1};
    Logger&                                logger;
    Scheduler&                             sched;
    int                                    timeQuantum;   // for legacy RR
    bool                                   kernelMode = false;

    // IPC
    std::map<int, std::vector<IPCMessage>> inbox;

    // Deadlock: resource → holding PID (-1 = free)
    std::map<std::string, int>              resourceHolder;
    std::map<int, std::vector<std::string>> processWaiting;
    std::mutex                              dlMtx;

    // -- Build task exe path -----------------------------
    std::string buildExePath(const std::string& taskName) {
        return "tasks\\" + taskName + ".exe";
    }

    // -- Launch real Windows child process ---------------
    //    Falls back to simulated mode if .exe not found
    bool spawnProcess(PCB& pcb) {
#ifdef _WIN32
        if (!pcb.exePath.empty() &&
            GetFileAttributesA(pcb.exePath.c_str()) == INVALID_FILE_ATTRIBUTES) {
            logger.warn("PROC", "Exe not found: " + pcb.exePath + " – simulated mode");
            pcb.winPID   = 0;
            pcb.hProcess = INVALID_HANDLE_VALUE;
            return true;
        }
        if (pcb.exePath.empty()) {
            pcb.winPID   = 0;
            pcb.hProcess = INVALID_HANDLE_VALUE;
            return true;
        }

        STARTUPINFOA si{};
        PROCESS_INFORMATION pi{};
        si.cb        = sizeof(si);
        si.dwFlags   = STARTF_USESTDHANDLES;
        si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
        si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);

        BOOL ok = CreateProcessA(
            nullptr,
            const_cast<char*>(pcb.exePath.c_str()),
            nullptr, nullptr, FALSE,
            CREATE_NEW_CONSOLE,
            nullptr, nullptr,
            &si, &pi
        );
        if (!ok) {
            logger.warn("PROC", "CreateProcess failed for " + pcb.exePath +
                        " (err=" + std::to_string(GetLastError()) + ") – simulated mode");
            pcb.winPID   = 0;
            pcb.hProcess = INVALID_HANDLE_VALUE;
            return true;
        }
        pcb.hProcess = pi.hProcess;
        pcb.hThread  = pi.hThread;
        pcb.winPID   = pi.dwProcessId;
        logger.info("PROC", "CreateProcess OK: " + pcb.exePath +
                    " winPID=" + std::to_string(pi.dwProcessId));
#endif
        return true;
    }

public:
    ProcessManager(Logger& log, Scheduler& sc, int quantum = 3)
        : logger(log), sched(sc), timeQuantum(quantum) {}

    // ==================================================
    //  Task 3: Create / Spawn Process
    //  Supports both legacy (burst/mem) and extended
    //  (TaskType, QueueLevel, ramMB, hddMB, cores) modes
    // ==================================================
    // -- Legacy create (original interface kept) --------
    int createProcess(const std::string& name, int burst,
                      int mem, const std::string& owner, int prio = 0) {
        std::lock_guard<std::mutex> lk(tableMtx);
        PCB p(nextPid++, name, burst, mem, owner, prio);
        p.state      = ProcessState::READY;
        p.startTime  = nowStr();
        p.exePath    = "";   // no exe for legacy-style processes
        processList.push_back(p);
        sched.enqueue(&processList.back());
        logger.info("PROC", "Created process [" + std::to_string(p.pid) +
                    "] '" + name + "' burst=" + std::to_string(burst) +
                    " mem=" + std::to_string(mem) + "KB");
        return p.pid;
    }

    // -- Extended create (with task type, queue, resources) -
    int spawnTask(const std::string& name,
                  TaskType type, QueueLevel ql,
                  int ramMB, int hddMB, int cores,
                  int priority, int burstMs,
                  const std::string& owner,
                  const std::string& exeName = "") {
        std::lock_guard<std::mutex> lk(tableMtx);
        PCB p;
        p.pid           = nextPid++;
        p.name          = name;
        p.type          = type;
        p.qLevel        = ql;
        p.ramMB         = ramMB;
        p.hddMB         = hddMB;
        p.coresUsed     = cores;
        p.priority      = priority;
        p.burstTime     = burstMs;
        p.remainingTime = burstMs;
        p.memoryRequired= ramMB * 1024;
        p.owner         = owner;
        p.exePath       = exeName.empty() ? buildExePath(name) : exeName;
        p.startTime     = nowStr();
        p.state         = ProcessState::NEW;

        // Determine queue from type if not overridden
        if (type == TaskType::AUTO)       p.qLevel = QueueLevel::LOW;
        else if (type == TaskType::BACKGROUND) p.qLevel = QueueLevel::MEDIUM;

        spawnProcess(p);
        processList.push_back(p);
        sched.enqueue(&processList.back());

        logger.info("PROC", "Spawned task PID=" + std::to_string(p.pid) +
                    " '" + name + "' type=" + taskTypeStr(type) +
                    " RAM=" + std::to_string(ramMB) + "MB");
        return p.pid;
    }

    // ==================================================
    //  Task 3: Terminate Process
    // ==================================================
    bool terminateProcess(int pid, bool force = false) {
        std::lock_guard<std::mutex> lk(tableMtx);
        for (auto& p : processList) {
            if (p.pid != pid) continue;
            if (p.state == ProcessState::TERMINATED) {
                std::cout << YELLOW << "  PID " << pid << " already terminated.\n" << RESET;
                return false;
            }
            if (!force && !kernelMode && p.state == ProcessState::RUNNING) {
                std::cout << RED << "  Switch to kernel mode to force-terminate a running process.\n" << RESET;
                return false;
            }
#ifdef _WIN32
            if (p.hProcess != INVALID_HANDLE_VALUE && p.hProcess != nullptr) {
                TerminateProcess(p.hProcess, 0);
                CloseHandle(p.hProcess);
                CloseHandle(p.hThread);
                p.hProcess = INVALID_HANDLE_VALUE;
            }
#endif
            sched.remove(pid);
            p.state = ProcessState::TERMINATED;
            logger.info("PROC", "PID " + std::to_string(pid) + " '" + p.name + "' terminated.");
            return true;
        }
        std::cout << RED << "  PID " << pid << " not found.\n" << RESET;
        return false;
    }

    // ==================================================
    //  Task 4: Show PCB details
    // ==================================================
    void showPCB(int pid) const {
        std::lock_guard<std::mutex> lk(tableMtx);
        for (const auto& p : processList) {
            if (p.pid != pid) continue;
            std::cout << BOLD << "\n  === PCB [PID " << pid << "] ===========================\n" << RESET;
            std::cout << "  |  Name      : " << p.name             << "\n";
            std::cout << "  |  State     : " << stateToStr(p.state)<< "\n";
            std::cout << "  |  Type      : " << taskTypeStr(p.type)<< "\n";
            std::cout << "  |  Queue     : " << (p.qLevel==QueueLevel::HIGH?"HIGH(RR)":
                                                  p.qLevel==QueueLevel::MEDIUM?"MED(FCFS)":"LOW(Priority)") << "\n";
            std::cout << "  |  Burst     : " << p.burstTime        << " ms\n";
            std::cout << "  |  Remaining : " << p.remainingTime    << " ms\n";
            std::cout << "  |  Wait      : " << p.waitingTime      << " ms\n";
            std::cout << "  |  TAT       : " << p.turnaroundTime   << " ms\n";
            std::cout << "  |  Memory    : " << p.memoryRequired   << " KB\n";
            std::cout << "  |  RAM       : " << p.ramMB            << " MB\n";
            std::cout << "  |  HDD       : " << p.hddMB            << " MB\n";
            std::cout << "  |  Cores     : " << p.coresUsed        << "\n";
            std::cout << "  |  Priority  : " << p.priority         << "\n";
            std::cout << "  |  Owner     : " << p.owner            << "\n";
            std::cout << "  |  Started   : " << p.startTime        << "\n";
#ifdef _WIN32
            std::cout << "  |  WinPID    : " << p.winPID           << "\n";
#endif
            std::cout << "  =============================================\n";
            return;
        }
        std::cout << RED << "  PCB not found for PID " << pid << RESET << "\n";
    }

    // ==================================================
    //  List all processes (ps)
    // ==================================================
    void listProcesses() const {
        std::lock_guard<std::mutex> lk(tableMtx);
        std::cout << BOLD
                  << "\n  PID   NAME             STATE       TYPE        QUEUE  MEM(KB)  OWNER\n"
                  << "  -----------------------------------------------------------------------\n"
                  << RESET;
        for (const auto& p : processList) {
            if (p.state == ProcessState::TERMINATED) continue;
            std::string sc = RESET;
            if      (p.state == ProcessState::RUNNING)   sc = GREEN;
            else if (p.state == ProcessState::BLOCKED)   sc = RED;
            else if (p.state == ProcessState::MINIMIZED) sc = YELLOW;
            else                                          sc = CYAN;
            std::string ql = (p.qLevel==QueueLevel::HIGH ? "HIGH " :
                              p.qLevel==QueueLevel::MEDIUM ? "MED  " : "LOW  ");
            std::cout << "  " << std::setw(5) << p.pid
                      << "  " << std::setw(15) << std::left << p.name
                      << "  " << sc << std::setw(10) << stateToStr(p.state) << RESET
                      << "  " << std::setw(10) << taskTypeStr(p.type)
                      << "  " << ql
                      << "  " << std::setw(7) << p.memoryRequired
                      << "  " << p.owner << "\n";
        }
        if (processList.empty())
            std::cout << YELLOW << "  No processes.\n" << RESET;
    }

    // ==================================================
    //  Task 5: FCFS Scheduling (simulated)
    // ==================================================
    void scheduleFCFS() {
        std::vector<PCB*> ready;
        for (auto& p : processList)
            if (p.state == ProcessState::READY)
                ready.push_back(&p);

        if (ready.empty()) {
            std::cout << YELLOW << "  No READY processes to schedule.\n" << RESET;
            return;
        }

        std::cout << BOLD << "\n  === FCFS Scheduling ===\n" << RESET;
        std::cout << BOLD
                  << "  Process       BT    WT    TAT\n"
                  << "  ------------------------------\n" << RESET;

        int currentTime = 0;
        float totalWT = 0, totalTAT = 0;

        for (auto* p : ready) {
            p->state          = ProcessState::RUNNING;
            p->waitingTime    = currentTime;
            currentTime      += p->burstTime;
            p->turnaroundTime = currentTime;
            p->remainingTime  = 0;
            p->state          = ProcessState::TERMINATED;
            totalWT  += p->waitingTime;
            totalTAT += p->turnaroundTime;

            std::cout << "  " << std::setw(12) << std::left << p->name
                      << "  " << std::setw(4) << p->burstTime
                      << "  " << std::setw(4) << p->waitingTime
                      << "  " << p->turnaroundTime << "\n";
            logger.info("SCHED", "FCFS: ran [" + p->name + "] WT=" +
                        std::to_string(p->waitingTime) + " TAT=" +
                        std::to_string(p->turnaroundTime));
        }

        int n = (int)ready.size();
        std::cout << GREEN
                  << "  Avg WT : " << totalWT  / n << " ms\n"
                  << "  Avg TAT: " << totalTAT / n << " ms\n"
                  << RESET;
    }

    // ==================================================
    //  Task 6: Round Robin Scheduling (simulated)
    // ==================================================
    void scheduleRR() {
        std::vector<PCB*> ready;
        for (auto& p : processList)
            if (p.state == ProcessState::READY) {
                p.remainingTime = p.burstTime;
                ready.push_back(&p);
            }

        if (ready.empty()) {
            std::cout << YELLOW << "  No READY processes to schedule.\n" << RESET;
            return;
        }

        std::cout << BOLD << "\n  === Round Robin (Quantum=" << timeQuantum << ") ===\n" << RESET;

        std::queue<PCB*> q;
        for (auto* p : ready) q.push(p);

        int currentTime = 0;
        std::map<int,int> startTime;
        for (auto* p : ready) startTime[p->pid] = -1;

        std::cout << BOLD << "  Gantt: ";
        while (!q.empty()) {
            PCB* p = q.front(); q.pop();
            if (p->state == ProcessState::TERMINATED) continue;
            if (startTime[p->pid] == -1) startTime[p->pid] = currentTime;

            int slice = std::min(timeQuantum, p->remainingTime);
            std::cout << "[" << p->name << ":" << slice << "] ";
            p->state          = ProcessState::RUNNING;
            currentTime      += slice;
            p->remainingTime -= slice;

            if (p->remainingTime == 0) {
                p->state          = ProcessState::TERMINATED;
                p->turnaroundTime = currentTime;
                p->waitingTime    = p->turnaroundTime - p->burstTime;
            } else {
                p->state = ProcessState::READY;
                q.push(p);
            }
        }
        std::cout << "\n\n";
        std::cout << BOLD
                  << "  Process       BT    WT    TAT\n"
                  << "  ------------------------------\n" << RESET;
        float totalWT = 0, totalTAT = 0;
        for (auto* p : ready) {
            totalWT  += p->waitingTime;
            totalTAT += p->turnaroundTime;
            std::cout << "  " << std::setw(12) << std::left << p->name
                      << "  " << std::setw(4) << p->burstTime
                      << "  " << std::setw(4) << p->waitingTime
                      << "  " << p->turnaroundTime << "\n";
        }
        int n = (int)ready.size();
        std::cout << GREEN
                  << "  Avg WT : " << totalWT  / n << " ms\n"
                  << "  Avg TAT: " << totalTAT / n << " ms\n"
                  << RESET;
        logger.info("SCHED", "Round Robin completed for " + std::to_string(n) + " processes.");
    }

    // ==================================================
    //  Multi-Level Queues: show status
    // ==================================================
    void showQueues() { sched.printQueues(); }

    // ==================================================
    //  Interrupt Simulation  (RUNNING → BLOCKED)
    // ==================================================
    bool interrupt(int pid) {
        std::lock_guard<std::mutex> lk(tableMtx);
        for (auto& p : processList) {
            if (p.pid == pid && p.state == ProcessState::RUNNING) {
                p.state = ProcessState::BLOCKED;
                sched.block(&p);
                logger.info("PROC", "Interrupt: PID " + std::to_string(pid) + " → BLOCKED");
                std::cout << YELLOW << "  PID " << pid << " interrupted → BLOCKED\n" << RESET;
                return true;
            }
        }
        std::cout << RED << "  PID " << pid << " is not RUNNING.\n" << RESET;
        return false;
    }

    // -- Resume after interrupt (BLOCKED → READY) -------
    bool resumeProc(int pid) {
        std::lock_guard<std::mutex> lk(tableMtx);
        for (auto& p : processList) {
            if (p.pid == pid && p.state == ProcessState::BLOCKED) {
                sched.resume(&p);
                logger.info("PROC", "PID " + std::to_string(pid) + " resumed from BLOCKED");
                std::cout << GREEN << "  PID " << pid << " resumed → READY\n" << RESET;
                return true;
            }
        }
        std::cout << RED << "  PID " << pid << " is not BLOCKED.\n" << RESET;
        return false;
    }

    // ==================================================
    //  Minimize / Restore
    // ==================================================
    bool minimize(int pid) {
        std::lock_guard<std::mutex> lk(tableMtx);
        for (auto& p : processList) {
            if (p.pid == pid && p.state != ProcessState::TERMINATED) {
                p.state = ProcessState::MINIMIZED;
                sched.block(&p);
                logger.info("PROC", "PID " + std::to_string(pid) + " minimized.");
                std::cout << GREEN << "  PID " << pid << " minimized (stays in memory).\n" << RESET;
                return true;
            }
        }
        return false;
    }

    bool restore(int pid) {
        std::lock_guard<std::mutex> lk(tableMtx);
        for (auto& p : processList) {
            if (p.pid == pid && p.state == ProcessState::MINIMIZED) {
                sched.resume(&p);
                logger.info("PROC", "PID " + std::to_string(pid) + " restored.");
                std::cout << GREEN << "  PID " << pid << " restored to READY queue.\n" << RESET;
                return true;
            }
        }
        std::cout << YELLOW << "  PID " << pid << " is not minimized.\n" << RESET;
        return false;
    }

    // ==================================================
    //  Kernel mode toggle
    // ==================================================
    void setKernelMode(bool on) {
        kernelMode = on;
        std::cout << (on ? MAGENTA : CYAN)
                  << "  Switched to " << (on ? "KERNEL" : "USER") << " mode.\n"
                  << RESET;
        logger.info("KERNEL", std::string("Entered ") + (on ? "KERNEL" : "USER") + " mode");
    }
    bool isKernelMode() const { return kernelMode; }

    // ==================================================
    //  Context Switch Simulation
    // ==================================================
    void contextSwitch(int fromPid, int toPid) {
        logger.info("CTX", "Context switch: PID " + std::to_string(fromPid) +
                    " → PID " + std::to_string(toPid));
        std::cout << MAGENTA << "  [Context Switch] PID " << fromPid
                  << " → PID " << toPid << RESET << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    // ==================================================
    //  Task 9: Deadlock Detection (resource-alloc graph)
    // ==================================================
    void acquireResource(int pid, const std::string& res) {
        std::lock_guard<std::mutex> lk(dlMtx);
        if (!resourceHolder.count(res) || resourceHolder[res] == -1) {
            resourceHolder[res] = pid;
            logger.info("DL", "PID " + std::to_string(pid) + " acquired '" + res + "'");
            std::cout << GREEN << "  PID " << pid << " acquired '" << res << "'\n" << RESET;
        } else {
            processWaiting[pid].push_back(res);
            logger.warn("DL", "PID " + std::to_string(pid) + " waiting for '" + res +
                        "' held by PID " + std::to_string(resourceHolder[res]));
            std::cout << YELLOW << "  PID " << pid << " waiting for '" << res
                      << "' (held by PID " << resourceHolder[res] << ")\n" << RESET;
        }
    }

    void releaseResource(int pid, const std::string& res) {
        std::lock_guard<std::mutex> lk(dlMtx);
        if (resourceHolder.count(res) && resourceHolder[res] == pid) {
            resourceHolder[res] = -1;
            for (auto& [wp, wl] : processWaiting) {
                auto it = std::find(wl.begin(), wl.end(), res);
                if (it != wl.end()) { wl.erase(it); resourceHolder[res] = wp; break; }
            }
            logger.info("DL", "PID " + std::to_string(pid) + " released '" + res + "'");
            std::cout << GREEN << "  PID " << pid << " released '" << res << "'\n" << RESET;
        } else {
            std::cout << RED << "  PID " << pid << " does not hold '" << res << "'\n" << RESET;
        }
    }

    void detectDeadlock() {
        std::lock_guard<std::mutex> lk(dlMtx);
        std::map<int,int> waitFor;
        for (const auto& [pid, rl] : processWaiting)
            for (const auto& r : rl) {
                auto it = resourceHolder.find(r);
                if (it != resourceHolder.end() && it->second != -1 && it->second != pid) {
                    waitFor[pid] = it->second; break;
                }
            }
        if (waitFor.empty()) {
            std::cout << GREEN << "  No deadlock detected. System is safe.\n" << RESET;
            return;
        }
        std::set<int> dead;
        for (auto& [s,_] : waitFor) {
            std::set<int> vis; int cur = s;
            while (waitFor.count(cur)) {
                if (vis.count(cur)) { dead.insert(cur); break; }
                vis.insert(cur); cur = waitFor[cur];
            }
        }
        if (!dead.empty()) {
            std::cout << RED << "  *** Deadlock detected among PIDs: ";
            for (int p : dead) std::cout << p << " ";
            std::cout << "***\n" << RESET;
            logger.error("DL", "DEADLOCK DETECTED");
        } else {
            std::cout << GREEN << "  No circular wait detected.\n" << RESET;
        }
    }

    void showResources() const {
        std::cout << BOLD << "\n  Resource Allocation:\n" << RESET;
        if (resourceHolder.empty()) {
            std::cout << YELLOW << "  No resources acquired.\n" << RESET;
            return;
        }
        for (const auto& [res, pid] : resourceHolder) {
            std::cout << "  " << std::setw(12) << std::left << res;
            if (pid == -1) std::cout << GREEN << "  FREE\n" << RESET;
            else           std::cout << YELLOW << "  held by PID " << pid << "\n" << RESET;
        }
        for (const auto& [pid, rl] : processWaiting) {
            if (rl.empty()) continue;
            std::cout << RED << "  PID " << pid << " waiting for: ";
            for (const auto& r : rl) std::cout << r << " ";
            std::cout << "\n" << RESET;
        }
    }

    // ==================================================
    //  Task 13: IPC – Send / Read messages
    // ==================================================
    bool sendMessage(int fromPid, int toPid, const std::string& content) {
        bool toExists = false;
        for (const auto& p : processList)
            if (p.pid == toPid) { toExists = true; break; }
        if (!toExists) {
            logger.warn("IPC", "sendMessage: target PID " + std::to_string(toPid) + " not found.");
            return false;
        }
        time_t t = time(nullptr);
        std::string ts = std::string(ctime(&t));
        if (!ts.empty() && ts.back() == '\n') ts.pop_back();
        inbox[toPid].push_back({fromPid, toPid, content, ts});
        logger.info("IPC", "Message sent PID " + std::to_string(fromPid) +
                    " → PID " + std::to_string(toPid));
        return true;
    }

    void readMessages(int pid) {
        auto it = inbox.find(pid);
        if (it == inbox.end() || it->second.empty()) {
            std::cout << YELLOW << "  No messages for PID " << pid << "\n" << RESET;
            return;
        }
        std::cout << BOLD << "\n  Inbox for PID " << pid << ":\n" << RESET;
        for (const auto& m : it->second)
            std::cout << "  From PID " << m.fromPid << "  [" << m.timestamp << "]\n"
                      << "    \"" << m.content << "\"\n";
    }

    // ==================================================
    //  Task 14: System Calls Simulation
    // ==================================================
    void syscall(const std::string& call, int pid) {
        std::cout << MAGENTA << "  [SYSCALL] " << RESET;
        if (call == "fork") {
            bool found = false;
            for (const auto& p : processList) {
                if (p.pid == pid) {
                    found = true;
                    int newPid = createProcess(p.name + "_fork", p.burstTime,
                                               p.memoryRequired, p.owner, p.priority);
                    std::cout << "fork() – cloned PID " << pid << " → new PID " << newPid << "\n";
                    break;
                }
            }
            if (!found) std::cout << "fork(): PID " << pid << " not found.\n";
        }
        else if (call == "exit") {
            terminateProcess(pid);
            std::cout << "exit() – PID " << pid << " terminated.\n";
        }
        else if (call == "wait") {
            for (auto& p : processList) {
                if (p.pid == pid) {
                    p.state = ProcessState::WAITING;
                    std::cout << "wait() – PID " << pid << " set to WAITING.\n";
                    logger.info("SYSCALL", "wait() on PID " + std::to_string(pid));
                    return;
                }
            }
            std::cout << "wait(): PID " << pid << " not found.\n";
        }
        else if (call == "getpid") {
            std::cout << "getpid() → " << pid << "\n";
        }
        else {
            std::cout << "Unknown syscall: " << call << "\n";
            logger.warn("SYSCALL", "Unknown call: " + call);
        }
    }

    // -- Helpers ----------------------------------------
    bool pidExists(int pid) const {
        for (const auto& p : processList)
            if (p.pid == pid) return true;
        return false;
    }
    std::vector<PCB>& getProcesses() { return processList; }
    void setQuantum(int q)           { timeQuantum = q; }
    int  getNextPid() const          { return nextPid.load(); }

    // -- GUI helper: snapshot of all processes ----------
    // Returns a copy so GUI can iterate without holding lock
    std::vector<PCB> getProcList() const {
        std::lock_guard<std::mutex> lk(tableMtx);
        return processList;
    }
};
