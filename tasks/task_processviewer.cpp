/*
 * ============================================================
 * task_processviewer.cpp  –  Task 15: Process Viewer
 * Lists real Windows processes with PID, memory, name.
 * Read-only view — like a simple Task Manager.
 * AUTO task type.
 * ============================================================
 */
#include <iostream>
#include <string>
#include <iomanip>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
using namespace std;

/*
 * listProcesses() – enumerates running processes using
 *   CreateToolhelp32Snapshot and displays name, PID, memory.
 */
void listProcesses(){
    HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if(snap==INVALID_HANDLE_VALUE){cout<<"\033[31m  Failed.\033[0m\n";return;}
    PROCESSENTRY32 pe{}; pe.dwSize=sizeof(pe);
    cout<<"\033[1m  PID       NAME                    MEM(KB)\033[0m\n";
    cout<<"  -----------------------------------------\n";
    if(Process32First(snap,&pe)){
        do{
            HANDLE ph=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,pe.th32ProcessID);
            SIZE_T mem=0;
            if(ph){PROCESS_MEMORY_COUNTERS pmc{};if(GetProcessMemoryInfo(ph,&pmc,sizeof(pmc)))mem=pmc.WorkingSetSize/1024;CloseHandle(ph);}
            cout<<"  "<<setw(8)<<pe.th32ProcessID<<"  "<<setw(24)<<left<<pe.szExeFile<<"  "<<mem<<"\n";
        }while(Process32Next(snap,&pe));
    }
    CloseHandle(snap);
}

int main(){
    HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode=0; GetConsoleMode(h,&mode);
    SetConsoleMode(h,mode|ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleTitle("Ultron OS – Process Viewer [auto]");
cout << "\033[1;36m\n";
cout << "====================================================\n";
cout << "            ULTRON OS :: PROCESS VIEWER             \n";
cout << "====================================================\n";
cout << "\033[0m";
        cout<<"  Commands: list, refresh, close\n\n";
    listProcesses();
    string line;
    while(true){
        cout<<"\n  pv> "; if(!getline(cin,line)||line=="close") break;
        if(line=="list"||line=="refresh") listProcesses();
    }
    cout<<"\033[33m  Process Viewer closed.\033[0m\n"; Sleep(800); return 0;
}
