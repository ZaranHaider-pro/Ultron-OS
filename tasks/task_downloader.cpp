/*
 * ============================================================
 * task_downloader.cpp  –  Task 14: File Downloader Simulator
 * Simulates a file download with progress bar. BACKGROUND task.
 * ============================================================
 */
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <string>
#include <windows.h>
using namespace std;

atomic<bool> cancel(false);

/*
 * progressBar() – renders an ASCII progress bar given percent.
 */
string progressBar(int pct, int w=30){
    int filled=pct*w/100;
    return "["+string(filled,'=')+(filled<w?">":"")+string(max(0,w-filled-1),' ')+"] "+to_string(pct)+"%";
}

void inputThread(){ string s; while(getline(cin,s)) if(s=="cancel"||s=="close"){cancel=true;break;} }

int main(){
    HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode=0; GetConsoleMode(h,&mode);
    SetConsoleMode(h,mode|ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleTitle("Ultron OS – Downloader [background]");
cout << "\033[1;36m\n";
cout << "====================================================\n";
cout << "              ULTRON OS :: DOWNLOADER               \n";
cout << "====================================================\n";
cout << "\033[0m";
    cout<<"  Filename: "; string fname; cin>>fname;
    cout<<"  Size (MB): "; int sz; cin>>sz; cin.ignore();
    cout<<"  Type 'cancel' to stop.\n\n";
    thread inp(inputThread);
    for(int i=0;i<=100&&!cancel;i+=2){
        cout<<"\r  Downloading "<<fname<<" "<<progressBar(i)<<"  "<<flush;
        this_thread::sleep_for(chrono::milliseconds(sz*20));
    }
    inp.detach();
    if(cancel) cout<<"\n\033[31m  Download cancelled.\033[0m\n";
    else        cout<<"\n\033[32m  Download complete: "<<fname<<" ("<<sz<<" MB).\033[0m\n";
    Sleep(1500); return 0;
}
