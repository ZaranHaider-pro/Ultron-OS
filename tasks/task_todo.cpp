/*
 * ============================================================
 * task_todo.cpp  –  Task 13: To-Do List Manager
 * Add, complete, delete tasks. Auto-saves to data/todo.txt.
 * ============================================================
 */
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <windows.h>
using namespace std;
struct Item{ string text; bool done; };

/*
 * load/save – persists todo list to data/todo.txt
 */
void save(const vector<Item>& items){
    CreateDirectoryA("data",nullptr);
    ofstream f("data\\todo.txt");
    for(auto& it:items) f<<(it.done?"1":"0")<<" "<<it.text<<"\n";
}
vector<Item> load(){
    vector<Item> items;
    ifstream f("data\\todo.txt"); string l;
    while(getline(f,l)) if(l.size()>2) items.push_back({l.substr(2),l[0]=='1'});
    return items;
}

int main(){
    HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode=0; GetConsoleMode(h,&mode);
    SetConsoleMode(h,mode|ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleTitle("Ultron OS – To-Do List");
    auto items=load();
cout << "\033[1;36m\n";
cout << "====================================================\n";
cout << "              ULTRON OS :: TO DO LIST               \n";
cout << "====================================================\n";
cout << "\033[0m";
        cout<<"  Commands: list, add <text>, done <#>, del <#>, close\n\n";
    string line;
    while(true){
        cout<<"  todo> "; if(!getline(cin,line)||line=="close") break;
        istringstream ss(line); string cmd; ss>>cmd;
        string rest; getline(ss,rest); if(!rest.empty()&&rest[0]==' ') rest=rest.substr(1);
        if(cmd=="list"){
            if(items.empty()){cout<<"  (no items)\n";continue;}
            for(size_t i=0;i<items.size();i++)
                cout<<"  "<<(i+1)<<". ["<<(items[i].done?"X":" ")<<"] "<<items[i].text<<"\n";
        } else if(cmd=="add"&&!rest.empty()){
            items.push_back({rest,false}); save(items);
            cout<<"\033[32m  Added.\033[0m\n";
        } else if(cmd=="done"&&!rest.empty()){
            int n=stoi(rest)-1;
            if(n>=0&&n<(int)items.size()){items[n].done=true;save(items);cout<<"\033[32m  Done!\033[0m\n";}
        } else if(cmd=="del"&&!rest.empty()){
            int n=stoi(rest)-1;
            if(n>=0&&n<(int)items.size()){items.erase(items.begin()+n);save(items);cout<<"\033[32m  Deleted.\033[0m\n";}
        } else cout<<"\033[31m  Unknown command.\033[0m\n";
    }
    save(items);
    cout<<"\033[33m  To-Do closed.\033[0m\n"; Sleep(800); return 0;
}
