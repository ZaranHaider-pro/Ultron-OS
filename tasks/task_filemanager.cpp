/*
 * ============================================================
 * task_filemanager.cpp  –  Task 4: File Manager
 * Create, delete, move, copy, check info of files in data/.
 * INTERACTIVE task.
 * ============================================================
 */
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <windows.h>
using namespace std;

/*
 * printHelp() – lists available file manager commands.
 */
void printHelp() {
    cout << "\033[33m  Commands:\033[0m\n"
         << "  ls              – list files\n"
         << "  create <name>   – create file\n"
         << "  delete <name>   – delete file\n"
         << "  copy <src> <dst>– copy file\n"
         << "  move <src> <dst>– move/rename\n"
         << "  info <name>     – show file info\n"
         << "  read <name>     – show contents\n"
         << "  write <f> <txt> – write text to file\n"
         << "  close           – exit\n";
}

int main() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0; GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleTitle("Ultron OS – File Manager");
    CreateDirectoryA("data", nullptr);

cout << "\033[1;36m\n";
cout << "====================================================\n";
cout << "              ULTRON OS :: FILE MANAGER             \n";
cout << "====================================================\n";
cout << "\033[0m";
    printHelp(); cout << "\n";

    string line;
    while (true) {
        cout << "  fm> ";
        if (!getline(cin, line) || line == "close") break;
        istringstream ss(line);
        string cmd, a, b;
        ss >> cmd >> a;
        getline(ss, b);
        if (!b.empty() && b[0] == ' ') b = b.substr(1);

        if (cmd == "ls") {
            WIN32_FIND_DATAA fd; HANDLE fh = FindFirstFileA("data\\*", &fd);
            if (fh == INVALID_HANDLE_VALUE) { cout << "  (empty)\n"; }
            else { do { if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                cout << "  " << fd.cFileName << "  (" << fd.nFileSizeLow << " bytes)\n";
            } while (FindNextFileA(fh, &fd)); FindClose(fh); }
        }
        else if (cmd == "create" && !a.empty()) {
            ofstream f("data\\" + a); f << "";
            cout << "\033[32m  Created: data\\" << a << "\033[0m\n";
        }
        else if (cmd == "delete" && !a.empty()) {
            DeleteFileA(("data\\" + a).c_str())
                ? cout << "\033[32m  Deleted.\033[0m\n"
                : cout << "\033[31m  Not found.\033[0m\n";
        }
        else if (cmd == "copy" && !a.empty() && !b.empty()) {
            CopyFileA(("data\\" + a).c_str(), ("data\\" + b).c_str(), FALSE)
                ? cout << "\033[32m  Copied.\033[0m\n"
                : cout << "\033[31m  Failed.\033[0m\n";
        }
        else if (cmd == "move" && !a.empty() && !b.empty()) {
            MoveFileA(("data\\" + a).c_str(), ("data\\" + b).c_str())
                ? cout << "\033[32m  Moved.\033[0m\n"
                : cout << "\033[31m  Failed.\033[0m\n";
        }
        else if (cmd == "info" && !a.empty()) {
            WIN32_FILE_ATTRIBUTE_DATA fi;
            if (GetFileAttributesExA(("data\\" + a).c_str(), GetFileExInfoStandard, &fi)) {
                ULONGLONG sz = ((ULONGLONG)fi.nFileSizeHigh << 32) | fi.nFileSizeLow;
                cout << "  File : data\\" << a << "\n  Size : " << sz << " bytes\n";
            } else cout << "\033[31m  Not found.\033[0m\n";
        }
        else if (cmd == "read" && !a.empty()) {
            ifstream f("data\\" + a); string l;
            if (!f) { cout << "\033[31m  Not found.\033[0m\n"; continue; }
            while (getline(f, l)) cout << "  " << l << "\n";
        }
        else if (cmd == "write" && !a.empty() && !b.empty()) {
            ofstream f("data\\" + a); f << b << "\n";
            cout << "\033[32m  Written.\033[0m\n";
        }
        else if (cmd == "help") printHelp();
        else cout << "\033[31m  Unknown command. Type 'help'.\033[0m\n";
    }
    cout << "\033[33m  File Manager closed.\033[0m\n";
    Sleep(800); return 0;
}
