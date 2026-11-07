/*
 * ============================================================
 * task_minesweeper.cpp  –  Task 5: Minesweeper Minigame
 * Classic Minesweeper on a 9x9 grid with 10 mines.
 * INTERACTIVE task (full-time user input).
 * ============================================================
 */
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
#include <windows.h>
using namespace std;

static const int ROWS = 9, COLS = 9, MINES = 10;
int  board[ROWS][COLS];   // -1 = mine, 0-8 = adjacent count
bool revealed[ROWS][COLS], flagged[ROWS][COLS];

/*
 * initBoard() – places MINES mines randomly and computes
 *               adjacent mine counts for every cell.
 */
void initBoard() {
    srand((unsigned)time(nullptr));
    for (int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++) board[r][c]=0,revealed[r][c]=false,flagged[r][c]=false;
    int placed=0;
    while(placed<MINES){
        int r=rand()%ROWS, c=rand()%COLS;
        if(board[r][c]!=-1){board[r][c]=-1;placed++;}
    }
    int dr[]={-1,-1,-1,0,0,1,1,1}, dc[]={-1,0,1,-1,1,-1,0,1};
    for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++) if(board[r][c]!=-1)
        for(int d=0;d<8;d++){int nr=r+dr[d],nc=c+dc[d];if(nr>=0&&nr<ROWS&&nc>=0&&nc<COLS&&board[nr][nc]==-1)board[r][c]++;}
}

void printBoard(bool revealAll=false) {
    cout << "\n     ";
    for(int c=0;c<COLS;c++) { cout<<c<<" "; } cout<<"\n     ";
    for(int c=0;c<COLS;c++) { cout<<"- "; } cout<<"\n";
    for(int r=0;r<ROWS;r++){
        cout<<"  "<<r<<" | ";
        for(int c=0;c<COLS;c++){
            if(flagged[r][c]&&!revealAll){cout<<"\033[33mF\033[0m ";}
            else if(!revealed[r][c]&&!revealAll){cout<<"# ";}
            else if(board[r][c]==-1){cout<<"\033[31m*\033[0m ";}
            else if(board[r][c]==0){cout<<". ";}
            else{cout<<"\033[36m"<<board[r][c]<<"\033[0m ";}
        }
        cout<<"\n";
    }
}

bool reveal(int r, int c) {
    if(r<0||r>=ROWS||c<0||c>=COLS||revealed[r][c]||flagged[r][c]) return true;
    revealed[r][c]=true;
    if(board[r][c]==-1) return false;
    if(board[r][c]==0){
        int dr[]={-1,-1,-1,0,0,1,1,1},dc[]={-1,0,1,-1,1,-1,0,1};
        for(int d=0;d<8;d++) reveal(r+dr[d],c+dc[d]);
    }
    return true;
}

int main(){
    HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode=0; GetConsoleMode(h,&mode);
    SetConsoleMode(h,mode|ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleTitle("Ultron OS – Minesweeper");
    initBoard();
cout << "\033[1;36m\n";
cout << "====================================================\n";
cout << "              ULTRON OS :: MINESWEEPER              \n";
cout << "====================================================\n";
cout << "\033[0m";
        cout<<"  Commands: r <row> <col>  f <row> <col>  close\n";
    int safe=ROWS*COLS-MINES, revealed_count=0;
    while(true){
        printBoard();
        cout<<"  Remaining: "<<safe-revealed_count<<" safe cells\n  > ";
        string line; if(!getline(cin,line)||line=="close") break;
        istringstream ss(line); string cmd; int r,c;
        ss>>cmd>>r>>c;
        if(cmd=="r"){
            if(!reveal(r,c)){cout<<"\033[31m  BOOM! You hit a mine!\033[0m\n";printBoard(true);break;}
            // count revealed
            revealed_count=0;
            for(int i=0;i<ROWS;i++) for(int j=0;j<COLS;j++) if(revealed[i][j]) revealed_count++;
            if(revealed_count==safe){cout<<"\033[32m  YOU WIN!\033[0m\n";printBoard(true);break;}
        } else if(cmd=="f"){
            if(!revealed[r][c]) flagged[r][c]=!flagged[r][c];
        }
    }
    cout<<"\033[33m  Minesweeper closed.\033[0m\n"; Sleep(1500); return 0;
}
