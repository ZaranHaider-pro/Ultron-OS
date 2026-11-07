/*
 * ============================================================
 * task_sysinfo.cpp  –  Task 7: System Info (AUTO task)
 * Displays memory, CPU, and OS information.
 * ============================================================
 */
#include <iostream>
#include <windows.h>
using namespace std;
int main(){
    HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode=0; GetConsoleMode(h,&mode);
    SetConsoleMode(h,mode|ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleTitle("Ultron OS – System Info");
    MEMORYSTATUSEX ms{}; ms.dwLength=sizeof(ms); GlobalMemoryStatusEx(&ms);
    SYSTEM_INFO si{}; GetSystemInfo(&si);
    char hostname[256]{}; DWORD hsz=256; GetComputerNameA(hostname,&hsz);
    char user[256]{}; DWORD usz=256; GetUserNameA(user,&usz);
cout << "\033[1;36m\n";
cout << "====================================================\n";
cout << "              ULTRON OS :: SYSTEM INFORMATION               \n";
cout << "====================================================\n";
cout << "\033[0m";
    cout<<"  Host    : "<<hostname<<"\n";
    cout<<"  User    : "<<user<<"\n";
    cout<<"  CPU     : "<<si.dwNumberOfProcessors<<" cores\n";
    cout<<"  RAM(tot): "<<ms.ullTotalPhys/1024/1024<<" MB\n";
    cout<<"  RAM(free): "<<ms.ullAvailPhys/1024/1024<<" MB\n";
    cout<<"  Load    : "<<ms.dwMemoryLoad<<"%\n";
    cout<<"\033[33m\n  Press Enter to close.\033[0m\n";
    string s; getline(cin,s);
    return 0;
}
