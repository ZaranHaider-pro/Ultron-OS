/*
 * ============================================================
 * task_print.cpp  –  Task 10: Print File (BACKGROUND task)
 * Simulates printing a file line by line with progress.
 * ============================================================
 */
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <windows.h>
using namespace std;
int main(){
    HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode=0; GetConsoleMode(h,&mode);
    SetConsoleMode(h,mode|ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleTitle("Ultron OS – Print [background]");
cout << "\033[1;36m\n";
cout << "====================================================\n";
cout << "              ULTRON OS :: PRINT FILE               \n";
cout << "====================================================\n";
cout << "\033[0m";
    cout<<"  File to print: "; string f; cin>>f; cin.ignore();
    ifstream file("data\\"+f);
    if(!file){cout<<"\033[31m  File not found.\033[0m\n"; Sleep(1500); return 1;}
    string line; int pg=1,lc=0;
    cout<<"\033[33m  Printing...\033[0m\n";
    while(getline(file,line)){
        lc++;
        cout<<"  ["<<lc<<"] "<<line<<"\n";
        this_thread::sleep_for(chrono::milliseconds(200));
        if(lc%20==0){cout<<"\033[36m  --- Page "<<pg++<<" ---\033[0m\n";}
    }
    cout<<"\033[32m  Print complete. "<<lc<<" lines printed.\033[0m\n";
    Sleep(1500); return 0;
}
