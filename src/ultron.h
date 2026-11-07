#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <queue>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <memory>
#include <functional>
#include <set>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <semaphore>
#include <chrono>
#include <thread>
#ifdef _WIN32
  #include <windows.h>
  #include <tlhelp32.h>
  #include <psapi.h>
#endif

// ---------------------------------------------
//  COLORS (Windows ANSI via virtual terminal)
// ---------------------------------------------
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"

// ---------------------------------------------
//  PROCESS STATES  (extended with BLOCKED/MINIMIZED)
// ---------------------------------------------
enum class ProcessState { NEW, READY, RUNNING, WAITING, BLOCKED, MINIMIZED, TERMINATED };

inline std::string stateToStr(ProcessState s) {
    switch (s) {
        case ProcessState::NEW:        return "NEW";
        case ProcessState::READY:      return "READY";
        case ProcessState::RUNNING:    return "RUNNING";
        case ProcessState::WAITING:    return "WAITING";
        case ProcessState::BLOCKED:    return "BLOCKED";
        case ProcessState::MINIMIZED:  return "MINIMIZED";
        case ProcessState::TERMINATED: return "TERMINATED";
    }
    return "UNKNOWN";
}

// ---------------------------------------------
//  TASK TYPES (for multi-level scheduling)
// ---------------------------------------------
enum class TaskType { INTERACTIVE, BACKGROUND, AUTO };

inline std::string taskTypeStr(TaskType t) {
    switch (t) {
        case TaskType::INTERACTIVE: return "INTERACTIVE";
        case TaskType::BACKGROUND:  return "BACKGROUND";
        case TaskType::AUTO:        return "AUTO";
    }
    return "?";
}

// ---------------------------------------------
//  QUEUE LEVELS for multi-level scheduler
// ---------------------------------------------
enum class QueueLevel { HIGH = 0, MEDIUM = 1, LOW = 2 };

// ---------------------------------------------
//  PROCESS CONTROL BLOCK (PCB)
// ---------------------------------------------
struct PCB {
    int          pid            = -1;
    std::string  name;
    ProcessState state          = ProcessState::NEW;
    TaskType     type           = TaskType::INTERACTIVE;
    QueueLevel   qLevel         = QueueLevel::MEDIUM;

    // Scheduling
    int  burstTime      = 10;
    int  remainingTime  = 10;
    int  waitingTime    = 0;
    int  turnaroundTime = 0;
    int  arrivalTime    = 0;
    int  priority       = 5;   // 1=highest, 10=lowest
    int  memoryRequired = 0;   // KB (legacy field, kept for compat)

    // Extended resources (MB)
    int  ramMB      = 0;
    int  hddMB      = 0;
    int  coresUsed  = 1;

    // Windows real-process handles
#ifdef _WIN32
    HANDLE hProcess = INVALID_HANDLE_VALUE;
    HANDLE hThread  = INVALID_HANDLE_VALUE;
    DWORD  winPID   = 0;
#endif

    std::string owner;
    std::string exePath;
    std::string startTime;

    // Legacy constructor (for compatibility with existing code)
    PCB() = default;
    PCB(int id, const std::string& n, int burst, int mem, const std::string& usr, int prio = 0)
        : pid(id), name(n), state(ProcessState::NEW),
          burstTime(burst), remainingTime(burst),
          waitingTime(0), turnaroundTime(0), arrivalTime(0),
          priority(prio), memoryRequired(mem), ramMB(mem/1024 > 0 ? mem/1024 : 1),
          owner(usr) {}
};

// ---------------------------------------------
//  MEMORY BLOCK
// ---------------------------------------------
struct MemoryBlock {
    int  start;
    int  size;
    bool isFree;
    int  allocatedPid;

    MemoryBlock(int s, int sz, bool free = true, int pid = -1)
        : start(s), size(sz), isFree(free), allocatedPid(pid) {}
};

// ---------------------------------------------
//  FILE SYSTEM NODES
// ---------------------------------------------
struct FSFile {
    std::string name;
    std::string content;
    std::string owner;
    std::string createdAt;
    bool        isDirectory;

    FSFile() : name(""), content(""), owner(""), isDirectory(false) {}

    FSFile(const std::string& n, const std::string& own, bool isDir = false)
        : name(n), content(""), owner(own), isDirectory(isDir) {
        time_t t = time(nullptr);
        createdAt = std::string(ctime(&t));
        if (!createdAt.empty() && createdAt.back() == '\n')
            createdAt.pop_back();
    }
};

// ---------------------------------------------
//  IPC MESSAGE
// ---------------------------------------------
struct IPCMessage {
    int         fromPid;
    int         toPid;
    std::string content;
    std::string timestamp;
};

// ---------------------------------------------
//  LOG ENTRY
// ---------------------------------------------
struct LogEntry {
    std::string level;
    std::string module;
    std::string message;
    std::string timestamp;
};

// ---------------------------------------------
//  SYSTEM RESOURCES SNAPSHOT
// ---------------------------------------------
struct SysResources {
    int totalRAM   = 2048;
    int freeRAM    = 2048;
    int totalHDD   = 256*1024;
    int freeHDD    = 256*1024;
    int totalCores = 8;
    int usedCores  = 0;
};

// ---------------------------------------------
//  UTILITY: timestamp strings
// ---------------------------------------------
inline std::string nowStr() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
    return std::string(buf);
}

inline std::string nowFullStr() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buf);
}
