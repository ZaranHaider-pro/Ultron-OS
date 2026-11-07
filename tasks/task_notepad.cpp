/*
 * ============================================================
 * task_notepad.cpp  –  Task 1: Notepad
 * Writes text to a file with auto-save every 10 seconds.
 * This is an INTERACTIVE task – runs until user types 'close'.
 * Each task is a separate process launched via CreateProcess.
 * ============================================================
 */
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <vector>
#include <sstream>
#include <windows.h>
using namespace std;

atomic<bool> running(true);
string       filename;
vector<string> lines;
mutex        fileMtx;

/*
 * autoSave() – background thread that saves file every 10 seconds.
 * Uses mutex to avoid race conditions with main write thread.
 */
void autoSave() {
    while (running) {
        this_thread::sleep_for(chrono::seconds(10));
        if (!running) break;
        lock_guard<mutex> lk(fileMtx);
        ofstream f("data\\" + filename);
        for (auto& l : lines) f << l << "\n";
        cout << "\n\033[33m  [AutoSave] Saved to data\\" << filename << "\033[0m\n> ";
        cout.flush();
    }
}

int main() {
    // Enable ANSI on this console window
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0; GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleTitle("Ultron OS – Notepad");
    CreateDirectoryA("data", nullptr);

    cout << "\033[33m\n  +==============================+\n"
         << "  |     Ultron OS – NOTEPAD       |\n"
         << "  +==============================+\033[0m\n";
    cout << "  Enter filename (no path): ";
    cin >> filename; cin.ignore();

    // Load existing content
    { ifstream f("data\\" + filename); string l; while (getline(f, l)) lines.push_back(l); }
    if (!lines.empty()) cout << "\033[36m  Loaded " << lines.size() << " existing lines.\033[0m\n";

    cout << "  Commands: 'save', 'show', 'del <#>', 'cls', 'close'\n\n";

    thread autoT(autoSave);
    string line;
    while (running) {
        cout << "> ";
        if (!getline(cin, line)) { running = false; break; }
        if (line == "close" || line == "exit") { running = false; break; }
        else if (line == "save") {
            lock_guard<mutex> lk(fileMtx);
            ofstream f("data\\" + filename);
            for (auto& l : lines) f << l << "\n";
            cout << "\033[32m  Saved (" << lines.size() << " lines).\033[0m\n";
        } else if (line == "show") {
            if (lines.empty()) { cout << "\033[33m  (empty)\033[0m\n"; }
            else for (size_t i = 0; i < lines.size(); i++)
                cout << "  " << (i+1) << ": " << lines[i] << "\n";
        } else if (line.substr(0,3) == "del") {
            istringstream ds(line); string dc; int dn; ds >> dc >> dn;
            dn--;
            lock_guard<mutex> lk(fileMtx);
            if (dn >= 0 && dn < (int)lines.size()) {
                lines.erase(lines.begin()+dn);
                cout << "\033[32m  Line deleted.\033[0m\n";
            } else cout << "\033[31m  Invalid line number.\033[0m\n";
        } else if (line == "cls") {
            lines.clear();
            cout << "\033[32m  Buffer cleared.\033[0m\n";
        } else {
            lock_guard<mutex> lk(fileMtx);
            lines.push_back(line);
        }
    }
    autoT.join();
    { ofstream f("data\\" + filename); for (auto& l : lines) f << l << "\n"; }
    cout << "\033[33m  Notepad closed. Final save done.\033[0m\n";
    Sleep(1500);
    return 0;
}
