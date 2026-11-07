/*
 * ============================================================
 * task_clock.cpp  –  Task 3: Live Clock (AUTO task)
 * Displays current date/time, updating every second.
 * Auto-started on OS boot. Type 'q' to close.
 * ============================================================
 */
#include <iostream>
#include <ctime>
#include <thread>
#include <chrono>
#include <atomic>
#include <string>
#include <windows.h>
using namespace std;

atomic<bool> quit(false);

/*
 * inputThread() – watches for 'q' input on this window
 * so the clock can be closed from its own terminal.
 */
void inputThread() {
    string s;
    while (getline(cin, s))
        if (s == "q" || s == "close") { quit = true; break; }
}

int main() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0; GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleTitle("Ultron OS – Clock [auto]");

    cout << "\033[35m\n  +===============================+\n"
         << "  |    Ultron OS – LIVE CLOCK      |\n"
         << "  +===============================+\033[0m\n"
         << "  Press 'q' + Enter to close.\n\n";

    thread it(inputThread);
    while (!quit) {
        time_t t = time(nullptr);
        tm tm2{}; localtime_s(&tm2, &t);
        char buf[64];
        strftime(buf, sizeof(buf), "  %A, %d %B %Y   %H:%M:%S", &tm2);
        cout << "\r\033[36m" << buf << "\033[0m   " << flush;
        this_thread::sleep_for(chrono::seconds(1));
    }
    it.detach();
    cout << "\n\033[33m  Clock closed.\033[0m\n";
    Sleep(800);
    return 0;
}
