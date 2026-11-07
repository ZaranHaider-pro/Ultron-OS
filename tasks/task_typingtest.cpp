/*
 * ============================================================
 * task_typingtest.cpp  –  Task 12: Typing Speed Test
 * Measures WPM and accuracy. INTERACTIVE task.
 * ============================================================
 */
#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <windows.h>
using namespace std;

/*
 * calcWPM() – computes words per minute given elapsed seconds
 *             and character count.
 */
double calcWPM(int chars, double secs){ return (chars/5.0)/(secs/60.0); }

int main(){
    HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode=0; GetConsoleMode(h,&mode);
    SetConsoleMode(h,mode|ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleTitle("Ultron OS – Typing Test");
    vector<string> sentences={
        "The quick brown fox jumps over the lazy dog",
        "Operating systems manage hardware and software",
        "A process is a program in execution",
        "Deadlock occurs when processes wait for each other",
        "Round robin scheduling gives equal CPU time"
    };
    srand((unsigned)time(nullptr));
    cout << "\033[1;36m\n";
cout << "====================================================\n";
cout << "              ULTRON OS :: TYPING TEST              \n";
cout << "====================================================\n";
cout << "\033[0m";
    cout<<"  Type 'close' to exit.\n\n";
    while(true){
        string target=sentences[rand()%sentences.size()];
        cout<<"  Type this:\n  \033[33m"<<target<<"\033[0m\n\n  > ";
        auto start=chrono::high_resolution_clock::now();
        string input; getline(cin,input);
        if(input=="close") break;
        auto end=chrono::high_resolution_clock::now();
        double secs=chrono::duration<double>(end-start).count();
        // Accuracy
        int correct=0;
        for(size_t i=0;i<min(target.size(),input.size());i++)
            if(target[i]==input[i]) correct++;
        double acc=100.0*correct/target.size();
        double wpm=calcWPM((int)input.size(),secs);
        cout<<"\033[32m  WPM     : "<<(int)wpm<<"\033[0m\n";
        cout<<"\033[32m  Accuracy: "<<(int)acc<<"%\033[0m\n\n";
    }
    cout<<"\033[33m  Typing test closed.\033[0m\n"; Sleep(800); return 0;
}
