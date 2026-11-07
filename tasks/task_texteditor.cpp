/*
 * ============================================================
 * task_texteditor.cpp  –  Task 9: Text Editor (line editor)
 * Full line-based editor with line numbers. INTERACTIVE.
 * ============================================================
 */
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <windows.h>
using namespace std;

/*
 * printLines() – shows buffer with line numbers.
 */
void printLines(const vector<string>& lines){
    for(size_t i=0;i<lines.size();i++)
        cout<<"  "<<(i+1)<<": "<<lines[i]<<"\n";
}

int main(){
    HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode=0; GetConsoleMode(h,&mode);
    SetConsoleMode(h,mode|ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleTitle("Ultron OS – Text Editor");
cout << "\033[1;36m\n";
cout << "====================================================\n";
cout << "              ULTRON OS :: TEXT EDITOR              \n";
cout << "====================================================\n";
cout << "\033[0m";
        cout<<"  Commands: open <f>, save <f>, add <text>,\n"
        <<"            del <line#>, show, close\n\n";
    vector<string> buf;
    string line, filename;
    while(true){
        cout<<"  ed> "; if(!getline(cin,line)||line=="close") break;
        istringstream ss(line); string cmd; ss>>cmd;
        string rest; getline(ss,rest); if(!rest.empty()&&rest[0]==' ') rest=rest.substr(1);
        if(cmd=="open"){
            filename=rest;
            ifstream f("data\\"+filename); buf.clear();
            string l; while(getline(f,l)) buf.push_back(l);
            cout<<"\033[32m  Opened ("<<buf.size()<<" lines).\033[0m\n";
        } else if(cmd=="save"){
            if(!rest.empty()) filename=rest;
            CreateDirectoryA("data",nullptr);
            ofstream f("data\\"+filename);
            for(auto& l:buf) f<<l<<"\n";
            cout<<"\033[32m  Saved to data\\"<<filename<<".\033[0m\n";
        } else if(cmd=="add"){
            buf.push_back(rest);
        } else if(cmd=="del"){
            int n=stoi(rest)-1;
            if(n>=0&&n<(int)buf.size()){buf.erase(buf.begin()+n);cout<<"\033[32m  Deleted.\033[0m\n";}
            else cout<<"\033[31m  Invalid line.\033[0m\n";
        } else if(cmd=="show"){
            printLines(buf);
        } else cout<<"\033[31m  Unknown command.\033[0m\n";
    }
    cout<<"\033[33m  Text Editor closed.\033[0m\n"; Sleep(800); return 0;
}
