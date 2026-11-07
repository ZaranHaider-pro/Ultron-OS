#pragma once
#include "ultron.h"
#include "logger.h"
#include <climits>
#include <set>

// =======================================================
//  MEMORY MANAGER
//  Task 7:  Memory Allocation – First Fit
//  Task 8:  Memory Allocation – Best Fit
//  Task 9:  Deadlock Detection (resource-allocation graph)
// =======================================================
class MemoryManager {
private:
    std::vector<MemoryBlock> blocks;
    int                      totalMemory;   // KB
    Logger&                  logger;

    // Deadlock: resource → holding PID
    std::map<std::string, int>              resourceHolder;  // resource → pid (-1=free)
    std::map<int, std::vector<std::string>> processWaiting;  // pid → [resources wanted]

    void mergeAdjacentFree() {
        for (size_t i = 0; i + 1 < blocks.size(); ) {
            if (blocks[i].isFree && blocks[i+1].isFree) {
                blocks[i].size += blocks[i+1].size;
                blocks.erase(blocks.begin() + i + 1);
            } else {
                i++;
            }
        }
    }

public:
    MemoryManager(int total, Logger& log) : totalMemory(total), logger(log) {
        blocks.push_back(MemoryBlock(0, total, true));
    }

    // --------------------------------------------------
    //  Display memory map
    // --------------------------------------------------
    void showMemory() const {
        std::cout << BOLD << "\n  Memory Map (Total: " << totalMemory << " KB)\n"
                  << "  +" << std::string(40, '-') << "+\n" << RESET;
        for (const auto& b : blocks) {
            std::string label = b.isFree ? "FREE" : "PID " + std::to_string(b.allocatedPid);
            std::string color = b.isFree ? GREEN : YELLOW;
            int barLen = std::max(1, b.size * 30 / totalMemory);
            std::cout << color
                      << "  | " << std::setw(8) << std::left << label
                      << " [" << std::string(barLen, b.isFree ? '.' : '#') << "]"
                      << "  start=" << std::setw(5) << b.start
                      << " size=" << b.size << " KB\n"
                      << RESET;
        }
        std::cout << BOLD << "  +" << std::string(40, '-') << "+\n" << RESET;
    }

    int  freeMemory() const {
        int f = 0;
        for (const auto& b : blocks) if (b.isFree) f += b.size;
        return f;
    }

    // ==================================================
    //  Task 7: First Fit
    // ==================================================
    bool firstFit(int pid, int size) {
        for (int i = 0; i < (int)blocks.size(); i++) {
            auto& b = blocks[i];
            if (b.isFree && b.size >= size) {
                if (b.size > size)
                    blocks.insert(blocks.begin() + i + 1,
                                  MemoryBlock(b.start + size, b.size - size, true));
                blocks[i].size         = size;
                blocks[i].isFree       = false;
                blocks[i].allocatedPid = pid;
                logger.info("MEM", "FirstFit: allocated " + std::to_string(size) +
                            " KB to PID " + std::to_string(pid) +
                            " at offset " + std::to_string(blocks[i].start));
                return true;
            }
        }
        logger.error("MEM", "FirstFit: no block large enough for " + std::to_string(size) + " KB (PID " + std::to_string(pid) + ")");
        return false;
    }

    // ==================================================
    //  Task 8: Best Fit
    // ==================================================
    bool bestFit(int pid, int size) {
        int bestIdx = -1, bestSize = INT_MAX;
        for (int i = 0; i < (int)blocks.size(); i++) {
            if (blocks[i].isFree && blocks[i].size >= size && blocks[i].size < bestSize) {
                bestSize = blocks[i].size;
                bestIdx  = i;
            }
        }
        if (bestIdx == -1) {
            logger.error("MEM", "BestFit: no suitable block for " + std::to_string(size) + " KB (PID " + std::to_string(pid) + ")");
            return false;
        }
        auto& b = blocks[bestIdx];
        if (b.size > size)
            blocks.insert(blocks.begin() + bestIdx + 1,
                          MemoryBlock(b.start + size, b.size - size, true));
        b.size         = size;
        b.isFree       = false;
        b.allocatedPid = pid;
        logger.info("MEM", "BestFit: allocated " + std::to_string(size) +
                    " KB to PID " + std::to_string(pid) +
                    " at offset " + std::to_string(b.start));
        return true;
    }

    // ==================================================
    //  Free memory for a PID
    // ==================================================
    bool freeMem(int pid) {
        bool freed = false;
        for (auto& b : blocks) {
            if (!b.isFree && b.allocatedPid == pid) {
                b.isFree       = true;
                b.allocatedPid = -1;
                freed          = true;
                logger.info("MEM", "Freed memory for PID " + std::to_string(pid) +
                            " (" + std::to_string(b.size) + " KB)");
            }
        }
        if (freed) mergeAdjacentFree();
        else logger.warn("MEM", "freeMem: no allocation found for PID " + std::to_string(pid));
        return freed;
    }

    // ==================================================
    //  Task 9: Deadlock Detection
    //  Simple resource-allocation graph cycle detection
    // ==================================================
    void acquireResource(int pid, const std::string& res) {
        if (!resourceHolder.count(res) || resourceHolder[res] == -1) {
            resourceHolder[res] = pid;
            logger.info("DL", "PID " + std::to_string(pid) + " acquired '" + res + "'");
        } else {
            processWaiting[pid].push_back(res);
            logger.warn("DL", "PID " + std::to_string(pid) + " waiting for '" + res +
                        "' held by PID " + std::to_string(resourceHolder[res]));
        }
    }

    void releaseResource(int pid, const std::string& res) {
        if (resourceHolder.count(res) && resourceHolder[res] == pid) {
            resourceHolder[res] = -1;
            logger.info("DL", "PID " + std::to_string(pid) + " released '" + res + "'");
            // Unblock any waiters
            for (auto& [wpid, wants] : processWaiting) {
                auto it = std::find(wants.begin(), wants.end(), res);
                if (it != wants.end()) {
                    wants.erase(it);
                    resourceHolder[res] = wpid;
                    logger.info("DL", "PID " + std::to_string(wpid) +
                                " acquired '" + res + "' after wait");
                    break;
                }
            }
        }
    }

    void detectDeadlock() {
        // Build wait-for graph and detect cycles via DFS
        std::map<int, int> waitFor;  // pid → pid it is waiting for
        for (const auto& [pid, resList] : processWaiting) {
            for (const auto& res : resList) {
                auto it = resourceHolder.find(res);
                if (it != resourceHolder.end() && it->second != -1 && it->second != pid) {
                    waitFor[pid] = it->second;
                    break;
                }
            }
        }

        if (waitFor.empty()) {
            std::cout << GREEN << "  No deadlock detected. System is safe.\n" << RESET;
            logger.info("DL", "Deadlock check: no deadlock.");
            return;
        }

        // Detect cycles
        std::vector<int> deadlocked;
        for (const auto& [start, _] : waitFor) {
            std::set<int> visited;
            int cur = start;
            while (waitFor.count(cur)) {
                if (visited.count(cur)) {
                    deadlocked.push_back(cur);
                    break;
                }
                visited.insert(cur);
                cur = waitFor[cur];
            }
        }

        if (deadlocked.empty()) {
            std::cout << GREEN << "  No circular wait detected.\n" << RESET;
        } else {
            std::cout << RED << "  ⚠ DEADLOCK DETECTED! Involved PIDs: ";
            for (int p : deadlocked) std::cout << p << " ";
            std::cout << "\n" << RESET;
            logger.error("DL", "Deadlock detected among " + std::to_string(deadlocked.size()) + " processes.");
            // Suggest resolution
            std::cout << YELLOW << "  Suggestion: terminate PID " << deadlocked[0]
                      << " to break the cycle.\n" << RESET;
        }
    }

    // -- GUI helpers ------------------------------------
    std::vector<MemoryBlock> getBlocks() const { return blocks; }
    int totalSize() const { return totalMemory; }

    void showResources() const {
        std::cout << BOLD << "\n  Resource Allocation:\n" << RESET;
        for (const auto& [res, pid] : resourceHolder) {
            if (pid == -1)
                std::cout << "  " << std::setw(10) << res << "  → FREE\n";
            else
                std::cout << "  " << std::setw(10) << res << "  → PID " << pid << "\n";
        }
        if (!processWaiting.empty()) {
            std::cout << YELLOW << "\n  Waiting Processes:\n" << RESET;
            for (const auto& [pid, rl] : processWaiting) {
                if (!rl.empty()) {
                    std::cout << "  PID " << pid << " waiting for: ";
                    for (const auto& r : rl) std::cout << r << " ";
                    std::cout << "\n";
                }
            }
        }
    }
};
