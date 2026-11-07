/*
 * ============================================================
 * task_music.cpp  –  Task 6: Music Player (BACKGROUND task)
 * Plays a beep melody in the background using Windows Beep().
 * Simulates a background song playing for a fixed duration.
 * Type 'stop' to end early.
 * ============================================================
 */
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <string>
#include <windows.h>
using namespace std;

atomic<bool> playing(true);

/*
 * playMelody() – plays a simple tune using Windows Beep().
 * Each Beep(freq, duration_ms) blocks for the given duration.
 */
void playMelody() {
    // Simple melody: frequency (Hz), duration (ms)
    int notes[][2] = {
        {523,300},{587,300},{659,300},{698,300},
        {784,400},{784,200},{880,400},{880,200},
        {784,600},{659,300},{587,300},{523,600},
        {440,300},{494,300},{523,400},{440,300},
        {392,600},{0,300},{523,300},{659,400},
        {784,600},{659,400},{523,600},{0,500}
    };
    int n = sizeof(notes)/sizeof(notes[0]);
    for (int i = 0; i < n && playing; i++) {
        if (notes[i][0] == 0)
            this_thread::sleep_for(chrono::milliseconds(notes[i][1]));
        else
            Beep(notes[i][0], notes[i][1]);
    }
    playing = false;
}

void inputThread() {
    string s;
    while (getline(cin, s))
        if (s == "stop" || s == "close") { playing = false; break; }
}

int main() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0; GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleTitle("Ultron OS – Music Player [background]");

cout << "\033[1;36m\n";
cout << "====================================================\n";
cout << "             ULTRON OS :: MUSIC PLAYER              \n";
cout << "====================================================\n";
cout << "\033[0m";
         cout<< "  Now playing: Ultron Melody\n"
         << "  Type 'stop' to end.\n\n";

    thread melody(playMelody);
    thread input(inputThread);

    while (playing) {
        this_thread::sleep_for(chrono::milliseconds(200));
    }
    melody.join();
    input.detach();
    cout << "\033[33m  Music stopped.\033[0m\n";
    Sleep(800);
    return 0;
}
