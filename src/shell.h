#pragma once
#include "ultron.h"
#include "logger.h"
#include "auth.h"
#include "process_manager.h"
#include "memory_manager.h"
#include "filesystem.h"
#include "scheduler.h"

// =======================================================
//  CLI SHELL  –  Task 1: Command Line Interface
//  Wires together all subsystems and handles user input
// =======================================================
class Shell {
private:
    Logger         logger;
    Scheduler      sched;
    AuthManager    auth;
    ProcessManager pm;
    MemoryManager  mm;
    FileSystem     fs;
    bool           running;

    // -- Banner -----------------------------------------
    void printBanner() const {
        std::cout << BOLD << CYAN
            << "\n"
            << "  ##+   ##+##+  ########+######+  ######+ ###+   ##+\n"
            << "  ##|   ##|##|  +==##+==+##+==##+##+===##+####+  ##|\n"
            << "  ##|   ##|##|     ##|   ######++##|   ##|##+##+ ##|\n"
            << "  ##|   ##|##|     ##|   ##+==##+##|   ##|##|+##+##|\n"
            << "  +######++#######+##|   ##|  ##|+######++##| +####|\n"
            << "   +=====+ +======++=+   +=+  +=+ +=====+ +=+  +===+\n"
            << RESET
            << YELLOW << "                   O P E R A T I N G   S Y S T E M\n"
            << RESET
            << "  Welcome Mr.Zaran Haider\n"
            << "  -----------------------------------------------------\n"
            << RESET;
    }

    // -- Prompt -----------------------------------------
    std::string prompt() const {
        std::string user  = auth.isLoggedIn() ? auth.getCurrentUser() : "guest";
        std::string color = auth.isRoot() ? RED : GREEN;
        std::string sym   = auth.isRoot() ? "#" : "$";
        std::string kmode = pm.isKernelMode() ? std::string(MAGENTA) + "[K]" + RESET : "";
        return color + user + RESET + "@" + CYAN + "ultron" + RESET
               + ":" + MAGENTA + fs.pwd() + RESET + kmode + sym + " ";
    }

    // -- Tokenise input ---------------------------------
    std::vector<std::string> split(const std::string& line) const {
        std::vector<std::string> tokens;
        std::istringstream iss(line);
        std::string tok;
        while (iss >> tok) tokens.push_back(tok);
        return tokens;
    }

    // -- Helpers ----------------------------------------
    void requireLogin() {
        if (!auth.isLoggedIn())
            std::cout << RED << "  You must be logged in. Use: login <user> <pass>\n" << RESET;
    }

    int toInt(const std::string& s, int def = 0) {
        try { return std::stoi(s); }
        catch (...) { return def; }
    }

    // ==================================================
    //  HELP
    // ==================================================
    void printHelp() const {
        std::cout << BOLD << "\n=====================================================================\n"
                  << "  |                       ULTRON OS COMMANDS                             |\n"
                  << "  =====================================================================\n" << RESET;

        auto row = [](const char* cmd, const char* desc) {
            std::cout << "  " << CYAN << std::setw(36) << std::left << cmd
                      << RESET << desc << "\n";
        };

        std::cout << YELLOW << "\n  -- Authentication ----------------------------------------------\n" << RESET;
        row("login <user> <pass>",          "Log in to the system");
        row("logout",                       "Log out current user");
        row("register <user> <pass>",       "Register new user");
        row("passwd <old> <new>",           "Change your password");
        row("users",                        "List all registered users");
        row("whoami",                       "Show current user");

        std::cout << YELLOW << "\n  -- Processes ---------------------------------------------------\n" << RESET;
        row("ps",                           "List all processes");
        row("spawn <name> <burst> <mem>",   "Create a new process (legacy)");
        row("task <name> <type> <ql> <ram> <hdd> <cores> <prio> <burst>",
                                            "Spawn a task (INTERACTIVE/BACKGROUND/AUTO, HIGH/MED/LOW)");
        std::cout << YELLOW << "\n  -- Launch Tasks (opens each in its own window) -------------\n" << RESET;
        row("launch notepad",       "Task  1: Notepad with auto-save");
        row("launch calculator",    "Task  2: Calculator (+  -  *  /  %)");
        row("launch clock",         "Task  3: Live clock [auto-start]");
        row("launch filemanager",   "Task  4: File Manager (create/del/copy/move)");
        row("launch minesweeper",   "Task  5: Minesweeper game");
        row("launch music",         "Task  6: Music player [background]");
        row("launch sysinfo",       "Task  7: System information [auto-start]");
        row("launch snake",         "Task  8: Snake game (WASD)");
        row("launch texteditor",    "Task  9: Line-based Text Editor");
        row("launch print",         "Task 10: Print file [background]");
        row("launch calendar",      "Task 11: Calendar [auto-start]");
        row("launch typingtest",    "Task 12: Typing speed test (WPM)");
        row("launch todo",          "Task 13: To-Do list manager");
        row("launch downloader",    "Task 14: File downloader [background]");
        row("launch processviewer", "Task 15: Process viewer [auto-start]");
        row("kill <pid>",                   "Terminate a process");
        row("pcb <pid>",                    "Show PCB of a process");
        row("fcfs",                         "Run FCFS scheduling on READY procs");
        row("rr [quantum]",                 "Run Round Robin scheduling");
        row("queues",                       "Show multi-level ready queues");
        row("interrupt <pid>",              "Simulate interrupt (RUNNING→BLOCKED)");
        row("resume <pid>",                 "Resume a BLOCKED process");
        row("minimize <pid>",               "Minimize a process (stays in RAM)");
        row("restore <pid>",                "Restore a minimized process");
        row("ctx <from> <to>",              "Simulate context switch");
        row("kmode",                        "Toggle kernel/user mode");
        row("msg <from> <to> <text...>",    "Send IPC message");
        row("inbox <pid>",                  "Read messages for a PID");
        row("syscall <call> <pid>",         "Simulate syscall (fork/exit/wait/getpid)");

        std::cout << YELLOW << "\n  -- Memory ------------------------------------------------------\n" << RESET;
        row("memmap",                       "Show memory map");
        row("alloc ff <pid> <size>",        "Allocate memory – First Fit");
        row("alloc bf <pid> <size>",        "Allocate memory – Best Fit");
        row("free <pid>",                   "Free memory for a PID");
        row("acquire <pid> <resource>",     "Acquire a resource (deadlock sim)");
        row("release <pid> <resource>",     "Release a resource");
        row("deadlock",                     "Run deadlock detection");
        row("resources",                    "Show resource allocation table");

        std::cout << YELLOW << "\n  -- File System -------------------------------------------------\n" << RESET;
        row("pwd",                          "Print working directory");
        row("ls [path]",                    "List directory contents");
        row("cd <path>",                    "Change directory");
        row("mkdir <path>",                 "Create directory");
        row("touch <path>",                 "Create empty file");
        row("write <path> <text...>",       "Write text to file (overwrite)");
        row("append <path> <text...>",      "Append text to file");
        row("cat <path>",                   "Read file contents");
        row("rm <path>",                    "Delete file or empty directory");
        row("cp <src> <dst>",               "Copy file");
        row("mv <src> <dst>",               "Move / rename file");

        std::cout << YELLOW << "\n  -- System ------------------------------------------------------\n" << RESET;
        row("log",                          "Show in-memory system log");
        row("syslog",                       "Show system.log file (last 30 lines)");
        row("clear",                        "Clear the screen");
        row("help",                         "Show this help");
        row("exit",                         "Shutdown Ultron OS");
        std::cout << "\n";
    }

    // ==================================================
    //  DISPATCH
    // ==================================================
    void dispatch(const std::vector<std::string>& t) {
        if (t.empty()) return;
        const std::string& cmd = t[0];

        // -- Authentication ----------------------------
        if (cmd == "login") {
            if (t.size() < 3) { std::cout << RED << "  Usage: login <user> <pass>\n" << RESET; return; }
            if (auth.login(t[1], t[2]))
                std::cout << GREEN << "  Welcome, " << t[1] << "! Type 'help' for commands.\n" << RESET;
            else
                std::cout << RED << "  Login failed.\n" << RESET;
        }
        else if (cmd == "logout") {
            auth.logout();
            std::cout << YELLOW << "  Logged out.\n" << RESET;
        }
        else if (cmd == "register") {
            if (t.size() < 3) { std::cout << RED << "  Usage: register <user> <pass>\n" << RESET; return; }
            if (auth.registerUser(t[1], t[2]))
                std::cout << GREEN << "  User '" << t[1] << "' registered.\n" << RESET;
        }
        else if (cmd == "passwd") {
            if (t.size() < 3) { std::cout << RED << "  Usage: passwd <old> <new>\n" << RESET; return; }
            auth.changePassword(t[1], t[2])
                ? std::cout << GREEN << "  Password updated.\n" << RESET
                : std::cout << RED << "  Failed.\n" << RESET;
        }
        else if (cmd == "users")  { auth.listUsers(); }
        else if (cmd == "whoami") {
            std::cout << "  " << (auth.isLoggedIn() ? auth.getCurrentUser() : "not logged in") << "\n";
        }

        // -- Processes --------------------------------
        else if (cmd == "ps")  { pm.listProcesses(); }
        else if (cmd == "spawn") {
            if (!auth.isLoggedIn()) { requireLogin(); return; }
            if (t.size() < 4) { std::cout << RED << "  Usage: spawn <name> <burst_ms> <mem_KB>\n" << RESET; return; }
            int pid = pm.createProcess(t[1], toInt(t[2]), toInt(t[3]), auth.getCurrentUser());
            std::cout << GREEN << "  Process created with PID " << pid << "\n" << RESET;
        }
        else if (cmd == "task") {
            // task <name> <type> <queue> <ramMB> <hddMB> <cores> <prio> <burst>
            if (!auth.isLoggedIn()) { requireLogin(); return; }
            if (t.size() < 9) {
                std::cout << RED << "  Usage: task <name> <INTERACTIVE|BACKGROUND|AUTO> <HIGH|MED|LOW>"
                          << " <ramMB> <hddMB> <cores> <priority> <burstMs>\n" << RESET; return;
            }
            // Parse type
            TaskType  type = TaskType::INTERACTIVE;
            if (t[2] == "BACKGROUND" || t[2] == "bg") type = TaskType::BACKGROUND;
            else if (t[2] == "AUTO"  || t[2] == "auto") type = TaskType::AUTO;
            // Parse queue
            QueueLevel ql = QueueLevel::MEDIUM;
            if (t[3] == "HIGH" || t[3] == "high") ql = QueueLevel::HIGH;
            else if (t[3] == "LOW" || t[3] == "low") ql = QueueLevel::LOW;

            int pid = pm.spawnTask(t[1], type, ql,
                                   toInt(t[4]), toInt(t[5]), toInt(t[6]),
                                   toInt(t[7]), toInt(t[8]),
                                   auth.getCurrentUser());
            if (pid > 0)
                std::cout << GREEN << "  Task spawned with PID " << pid << "\n" << RESET;
        }
        else if (cmd == "kill") {
            if (t.size() < 2) { std::cout << RED << "  Usage: kill <pid>\n" << RESET; return; }
            pm.terminateProcess(toInt(t[1]))
                ? std::cout << GREEN << "  Process killed.\n" << RESET
                : std::cout << RED << "  Failed.\n" << RESET;
        }
        else if (cmd == "pcb") {
            if (t.size() < 2) { std::cout << RED << "  Usage: pcb <pid>\n" << RESET; return; }
            pm.showPCB(toInt(t[1]));
        }
        else if (cmd == "fcfs") { pm.scheduleFCFS(); }
        else if (cmd == "rr") {
            if (t.size() >= 2) pm.setQuantum(toInt(t[1]));
            pm.scheduleRR();
        }
        else if (cmd == "queues") { pm.showQueues(); }
        else if (cmd == "interrupt") {
            if (t.size() < 2) { std::cout << RED << "  Usage: interrupt <pid>\n" << RESET; return; }
            pm.interrupt(toInt(t[1]));
        }
        else if (cmd == "resume") {
            if (t.size() < 2) { std::cout << RED << "  Usage: resume <pid>\n" << RESET; return; }
            pm.resumeProc(toInt(t[1]));
        }
        else if (cmd == "minimize") {
            if (t.size() < 2) { std::cout << RED << "  Usage: minimize <pid>\n" << RESET; return; }
            pm.minimize(toInt(t[1]));
        }
        else if (cmd == "restore") {
            if (t.size() < 2) { std::cout << RED << "  Usage: restore <pid>\n" << RESET; return; }
            pm.restore(toInt(t[1]));
        }
        else if (cmd == "ctx") {
            if (t.size() < 3) { std::cout << RED << "  Usage: ctx <fromPID> <toPID>\n" << RESET; return; }
            pm.contextSwitch(toInt(t[1]), toInt(t[2]));
        }
        else if (cmd == "kmode") {
            pm.setKernelMode(!pm.isKernelMode());
        }
        else if (cmd == "msg") {
            if (t.size() < 4) { std::cout << RED << "  Usage: msg <fromPID> <toPID> <text...>\n" << RESET; return; }
            std::string content;
            for (size_t i = 3; i < t.size(); i++) content += (i>3?" ":"") + t[i];
            pm.sendMessage(toInt(t[1]), toInt(t[2]), content)
                ? std::cout << GREEN << "  Message sent.\n" << RESET
                : std::cout << RED << "  Failed.\n" << RESET;
        }
        else if (cmd == "inbox") {
            if (t.size() < 2) { std::cout << RED << "  Usage: inbox <pid>\n" << RESET; return; }
            pm.readMessages(toInt(t[1]));
        }
        else if (cmd == "syscall") {
            if (t.size() < 3) { std::cout << RED << "  Usage: syscall <call> <pid>\n" << RESET; return; }
            pm.syscall(t[1], toInt(t[2]));
        }

        // -- Memory -----------------------------------
        else if (cmd == "memmap") { mm.showMemory(); }
        else if (cmd == "alloc") {
            if (t.size() < 4) { std::cout << RED << "  Usage: alloc <ff|bf> <pid> <size_KB>\n" << RESET; return; }
            int pid = toInt(t[2]), size = toInt(t[3]);
            bool ok = (t[1] == "ff") ? mm.firstFit(pid, size) : mm.bestFit(pid, size);
            ok ? std::cout << GREEN << "  Allocated " << size << " KB to PID " << pid << "\n" << RESET
               : std::cout << RED << "  Allocation failed.\n" << RESET;
        }
        else if (cmd == "free") {
            if (t.size() < 2) { std::cout << RED << "  Usage: free <pid>\n" << RESET; return; }
            mm.freeMem(toInt(t[1]))
                ? std::cout << GREEN << "  Memory freed.\n" << RESET
                : std::cout << RED << "  Failed.\n" << RESET;
        }
        else if (cmd == "acquire") {
            if (t.size() < 3) { std::cout << RED << "  Usage: acquire <pid> <resource>\n" << RESET; return; }
            pm.acquireResource(toInt(t[1]), t[2]);
        }
        else if (cmd == "release") {
            if (t.size() < 3) { std::cout << RED << "  Usage: release <pid> <resource>\n" << RESET; return; }
            pm.releaseResource(toInt(t[1]), t[2]);
        }
        else if (cmd == "deadlock")  { pm.detectDeadlock(); }
        else if (cmd == "resources") { pm.showResources(); }

        // -- File System ------------------------------
        else if (cmd == "pwd")   { std::cout << "  " << fs.pwd() << "\n"; }
        else if (cmd == "ls")    { fs.ls(t.size() > 1 ? t[1] : ""); }
        else if (cmd == "cd")    {
            if (t.size() < 2) { std::cout << RED << "  Usage: cd <path>\n" << RESET; return; }
            if (!fs.cd(t[1]))
                std::cout << RED << "  No such directory.\n" << RESET;
        }
        else if (cmd == "mkdir") {
            if (!auth.isLoggedIn()) { requireLogin(); return; }
            if (t.size() < 2) { std::cout << RED << "  Usage: mkdir <path>\n" << RESET; return; }
            fs.mkdir(t[1], auth.getCurrentUser())
                ? std::cout << GREEN << "  Directory created.\n" << RESET
                : std::cout << RED << "  Failed.\n" << RESET;
        }
        else if (cmd == "touch") {
            if (!auth.isLoggedIn()) { requireLogin(); return; }
            if (t.size() < 2) { std::cout << RED << "  Usage: touch <path>\n" << RESET; return; }
            fs.touch(t[1], auth.getCurrentUser());
            std::cout << GREEN << "  File created.\n" << RESET;
        }
        else if (cmd == "write" || cmd == "append") {
            if (!auth.isLoggedIn()) { requireLogin(); return; }
            if (t.size() < 3) { std::cout << RED << "  Usage: " << cmd << " <path> <text...>\n" << RESET; return; }
            std::string content;
            for (size_t i = 2; i < t.size(); i++) content += (i>2?" ":"") + t[i];
            fs.write(t[1], content, auth.getCurrentUser(), cmd == "append")
                ? std::cout << GREEN << "  Done.\n" << RESET
                : std::cout << RED << "  Failed.\n" << RESET;
        }
        else if (cmd == "cat") {
            if (t.size() < 2) { std::cout << RED << "  Usage: cat <path>\n" << RESET; return; }
            fs.read(t[1]);
        }
        else if (cmd == "rm") {
            if (!auth.isLoggedIn()) { requireLogin(); return; }
            if (t.size() < 2) { std::cout << RED << "  Usage: rm <path>\n" << RESET; return; }
            fs.rm(t[1], auth.getCurrentUser())
                ? std::cout << GREEN << "  Removed.\n" << RESET
                : std::cout << RED << "  Failed.\n" << RESET;
        }
        else if (cmd == "cp") {
            if (!auth.isLoggedIn()) { requireLogin(); return; }
            if (t.size() < 3) { std::cout << RED << "  Usage: cp <src> <dst>\n" << RESET; return; }
            fs.cp(t[1], t[2], auth.getCurrentUser())
                ? std::cout << GREEN << "  Copied.\n" << RESET
                : std::cout << RED << "  Failed.\n" << RESET;
        }
        else if (cmd == "mv") {
            if (!auth.isLoggedIn()) { requireLogin(); return; }
            if (t.size() < 3) { std::cout << RED << "  Usage: mv <src> <dst>\n" << RESET; return; }
            fs.mv(t[1], t[2], auth.getCurrentUser())
                ? std::cout << GREEN << "  Moved.\n" << RESET
                : std::cout << RED << "  Failed.\n" << RESET;
        }

        // -- System -----------------------------------
        else if (cmd == "log")    { logger.showAll(); }
        else if (cmd == "syslog") { logger.showLog(); }
        else if (cmd == "clear")  { system("cls"); }
        else if (cmd == "help")   { printHelp(); }
        else if (cmd == "exit" || cmd == "shutdown") {
            std::cout << YELLOW << "\n  Shutting down Ultron OS... Goodbye!\n" << RESET;
            running = false;
        }
        else if (cmd == "launch" || cmd == "run") {
            if (t.size() < 2) {
                std::cout << RED << "  Usage: launch <taskname>\n"
                          << "  Tasks: notepad calculator clock filemanager minesweeper\n"
                          << "         music sysinfo snake texteditor print calendar\n"
                          << "         typingtest todo downloader processviewer\n" << RESET;
                return;
            }
            launchTask(t[1]);
        }
                else {
            std::cout << RED << "  '" << cmd << "': command not found. Type 'help'.\n" << RESET;
            logger.warn("CLI", "Unknown command: " + cmd);
        }
    }

public:
    Shell() : logger(false), sched(logger), auth(logger),
              pm(logger, sched), mm(65536, logger), fs(logger), running(true) {
        // Enable ANSI on Windows
        #ifdef _WIN32
        system("chcp 65001 > nul");
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        GetConsoleMode(hOut, &mode);
        SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        #endif
    }


    // -- Auto-start tasks on boot (clock + calendar) ------------
    void autoStart() {
        std::cout << GREEN << "  [BOOT] Auto-starting: clock, calendar...\n" << RESET;
        pm.spawnTask("task_clock",    TaskType::AUTO, QueueLevel::LOW,
                     32, 10, 1, 8, 999, "system", "tasks\\task_clock.exe");
        pm.spawnTask("task_calendar", TaskType::AUTO, QueueLevel::LOW,
                     32, 10, 1, 9, 20,  "system", "tasks\\task_calendar.exe");
        std::cout << GREEN << "  [BOOT] Clock and Calendar launched.\n\n" << RESET;
    }

    // -- Launch a named task by shorthand name -------------------
    void launchTask(const std::string& name) {
        struct Cfg { TaskType type; QueueLevel ql; int ram,hdd,cores,prio,burst; };
        std::map<std::string,Cfg> cfg = {
            {"notepad",       {TaskType::INTERACTIVE, QueueLevel::HIGH,    64,  50, 1, 3,  30}},
            {"calculator",    {TaskType::INTERACTIVE, QueueLevel::HIGH,    32,  10, 1, 2,  20}},
            {"clock",         {TaskType::AUTO,        QueueLevel::LOW,     32,  10, 1, 8, 999}},
            {"filemanager",   {TaskType::INTERACTIVE, QueueLevel::HIGH,    64, 100, 1, 3,  40}},
            {"minesweeper",   {TaskType::INTERACTIVE, QueueLevel::HIGH,   128,  20, 2, 4,  60}},
            {"music",         {TaskType::BACKGROUND,  QueueLevel::MEDIUM,  64,  50, 1, 6, 120}},
            {"sysinfo",       {TaskType::AUTO,        QueueLevel::LOW,     32,  10, 1, 7,  15}},
            {"snake",         {TaskType::INTERACTIVE, QueueLevel::HIGH,   128,  20, 2, 4,  60}},
            {"texteditor",    {TaskType::INTERACTIVE, QueueLevel::HIGH,    64,  50, 1, 3,  30}},
            {"print",         {TaskType::BACKGROUND,  QueueLevel::MEDIUM,  32, 200, 1, 5,  80}},
            {"calendar",      {TaskType::AUTO,        QueueLevel::LOW,     32,  10, 1, 9,  20}},
            {"typingtest",    {TaskType::INTERACTIVE, QueueLevel::HIGH,    64,  20, 1, 3,  30}},
            {"todo",          {TaskType::INTERACTIVE, QueueLevel::MEDIUM,  32,  50, 1, 4,  25}},
            {"downloader",    {TaskType::BACKGROUND,  QueueLevel::MEDIUM,  64, 500, 1, 5, 100}},
            {"processviewer", {TaskType::AUTO,        QueueLevel::LOW,     64,  20, 1, 7,  20}},
        };
        if (!cfg.count(name)) {
            std::cout << RED << "  Unknown task \'" << name << "\'.\n"
                      << "  Available: notepad calculator clock filemanager minesweeper\n"
                      << "             music sysinfo snake texteditor print calendar\n"
                      << "             typingtest todo downloader processviewer\n" << RESET;
            return;
        }
        auto& c = cfg[name];
        std::string owner = auth.isLoggedIn() ? auth.getCurrentUser() : "system";
        int pid = pm.spawnTask("task_"+name, c.type, c.ql,
                               c.ram, c.hdd, c.cores, c.prio, c.burst,
                               owner, "tasks\\task_"+name+".exe");
        if (pid > 0)
            std::cout << GREEN << "  Launched \'" << name << "\' as PID " << pid << "\n" << RESET;
    }

    void run() {
        printBanner();
        std::cout << "  Type 'help' for commands. Login with: login root root123\n\n";

        // Auto-start clock and calendar
        autoStart();

        std::string line;
        while (running) {
            std::cout << prompt();
            if (!std::getline(std::cin, line)) break;

            while (!line.empty() && (line.front() == ' ' || line.front() == '\t')) line.erase(0,1);
            while (!line.empty() && (line.back()  == ' ' || line.back()  == '\t')) line.pop_back();
            if (line.empty()) continue;

            dispatch(split(line));
        }
    }
};
