#pragma once
#include "ultron.h"

// =======================================================
//  LOGGER
//  Task 15: Error Handling & Logging System
//  Enhanced: writes timestamped entries to console AND
//            logs/system.log on disk (from complete proj)
// =======================================================
class Logger {
private:
    std::vector<LogEntry> entries;
    bool                  verbose;
    std::mutex            mtx;
    std::ofstream         logFile;

    std::string timestamp() const {
        return "[" + nowFullStr() + "]";
    }

    void write(const std::string& level, const std::string& mod,
               const std::string& msg) {
        std::lock_guard<std::mutex> lk(mtx);

        // Build log line
        std::string line = timestamp() + " [" + level + "] (" + mod + ") " + msg;

        // Store in memory
        LogEntry e;
        e.level     = level;
        e.module    = mod;
        e.message   = msg;
        e.timestamp = nowFullStr();
        entries.push_back(e);

        // Write to file
        if (logFile.is_open()) {
            logFile << line << "\n";
            logFile.flush();
        }

        // Optionally print to console
        if (verbose) {
            std::string color = RESET;
            if      (level == "ERROR") color = RED;
            else if (level == "WARN")  color = YELLOW;
            else if (level == "INFO")  color = CYAN;
            std::cout << color << line << RESET << "\n";
        }
    }

public:
    // verbose=false by default so logs don't flood the shell
    Logger(bool v = false, const std::string& path = "logs/system.log")
        : verbose(v) {
#ifdef _WIN32
        CreateDirectoryA("logs", nullptr);
#endif
        logFile.open(path, std::ios::app);
        if (logFile.is_open())
            logFile << "\n===== ULTRON OS Session " << nowFullStr() << " =====\n";
    }

    ~Logger() {
        if (logFile.is_open()) logFile.close();
    }

    void info (const std::string& mod, const std::string& msg) { write("INFO",  mod, msg); }
    void warn (const std::string& mod, const std::string& msg) { write("WARN",  mod, msg); }
    void error(const std::string& mod, const std::string& msg) { write("ERROR", mod, msg); }

    // -- Show all log entries (original showAll kept) ----
    void showAll() const {
        std::cout << BOLD << "\n  +== System Log (" << entries.size() << " entries) ==+\n" << RESET;
        for (const auto& e : entries) {
            std::string col = CYAN;
            if      (e.level == "ERROR") col = RED;
            else if (e.level == "WARN")  col = YELLOW;
            std::cout << col
                      << "  [" << e.timestamp << "] "
                      << "[" << e.level << "] "
                      << "(" << e.module << ") "
                      << e.message << RESET << "\n";
        }
        if (entries.empty())
            std::cout << YELLOW << "  No log entries.\n" << RESET;
    }

    // -- Show last N entries from log file ---------------
    void showLog(int lines = 30) {
        std::ifstream f("logs/system.log");
        if (!f) { std::cout << YELLOW << "  No log file found.\n" << RESET; return; }
        std::vector<std::string> all;
        std::string ln;
        while (std::getline(f, ln)) all.push_back(ln);
        int start = std::max(0, (int)all.size() - lines);
        std::cout << BOLD << "\n  ===== System Log (last " << lines << " entries) =====\n" << RESET;
        for (int i = start; i < (int)all.size(); i++) {
            std::string col = CYAN;
            if (all[i].find("[ERROR]") != std::string::npos) col = RED;
            else if (all[i].find("[WARN]")  != std::string::npos) col = YELLOW;
            std::cout << col << "  " << all[i] << RESET << "\n";
        }
    }

    std::vector<LogEntry>& getEntries() { return entries; }
};
