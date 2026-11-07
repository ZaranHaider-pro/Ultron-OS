/*
 * ============================================================
 * task_snake.cpp  –  Task 8: Snake Game
 * Classic snake on a 20x20 grid. WASD to move.
 * Full-time interactive task – runs until close.
 * ============================================================
 */
#include <iostream>
#include <deque>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <atomic>
#include <chrono>
#include <windows.h>
#include <mutex>
#include <conio.h>
using namespace std;

const int W=20, H=20;
struct P{int x,y;};
deque<P> snake;
P food;
int dx=1,dy=0;
atomic<bool> alive(true);
mutex gmtx;

void placeFood(){
    srand((unsigned)time(nullptr)+rand());
    food={rand()%W, rand()%H};
}

void draw(){
    COORD c{0,6}; SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),c);
    string grid[H]; for(auto& r:grid) r=string(W,'.');
    for(auto& s:snake) if(s.x>=0&&s.x<W&&s.y>=0&&s.y<H) grid[s.y][s.x]='O';
    if(snake.size()) grid[snake.front().y][snake.front().x]='@';
    grid[food.y][food.x]='*';
    for(int r=0;r<H;r++) cout<<"  |"<<grid[r]<<"|\n";
    cout<<"  Score: "<<snake.size()-1<<"   \n";
}

void inputLoop(){
    while(alive){
        if(_kbhit()){
            char ch=_getch();
            lock_guard<mutex> lk(gmtx);
            if((ch=='w'||ch=='W')&&dy!=1)  {dx=0;dy=-1;}
            if((ch=='s'||ch=='S')&&dy!=-1) {dx=0;dy=1;}
            if((ch=='a'||ch=='A')&&dx!=1)  {dx=-1;dy=0;}
            if((ch=='d'||ch=='D')&&dx!=-1) {dx=1;dy=0;}
            if(ch=='q') alive=false;
        }
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

int main(){
    HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode=0; GetConsoleMode(h,&mode);
    SetConsoleMode(h,mode|ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleTitle("Ultron OS – Snake");
    CONSOLE_CURSOR_INFO ci{1,FALSE}; SetConsoleCursorInfo(h,&ci);
    snake.push_back({W/2,H/2}); placeFood();
    cout<<"\033[32m\n  +==============================+\n"
        <<"  |     Ultron OS – SNAKE         |\n"
        <<"  +==============================+\033[0m\n";
    cout<<"  WASD=move  q=quit\n";
    // print border top
    cout<<"  +"<<string(W,'-')<<"+\n";
    for(int r=0;r<H;r++) cout<<"  |"<<string(W,'.')<<"|\n";
    cout<<"  +"<<string(W,'-')<<"+\n";
    thread inp(inputLoop);
    while(alive){
        {
            // Lock only for the update step, NOT during sleep
            lock_guard<mutex> lk(gmtx);
            P head=snake.front();
            head.x+=dx; head.y+=dy;
            if(head.x<0||head.x>=W||head.y<0||head.y>=H){alive=false;break;}
            bool selfHit=false;
            for(auto& s:snake) if(s.x==head.x&&s.y==head.y){selfHit=true;break;}
            if(selfHit){alive=false;break;}
            snake.push_front(head);
            if(head.x==food.x&&head.y==food.y) placeFood();
            else snake.pop_back();
            draw();
        }
        // Sleep OUTSIDE lock so input thread can update direction freely
        this_thread::sleep_for(chrono::milliseconds(150));
    }
    inp.detach();
    cout<<"\033[31m\n  Game Over! Score: "<<snake.size()-1<<"\033[0m\n";
    cout<<"\033[33m  Press Enter to close.\033[0m\n";
    cin.ignore(); return 0;
}
