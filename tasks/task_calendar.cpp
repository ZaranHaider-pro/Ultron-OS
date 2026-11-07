/*
 * ============================================================
 * task_calendar.cpp  –  Task 11: Calendar (AUTO task)
 * Displays current month calendar. Auto-started at boot.
 * ============================================================
 */
#include <iostream>
#include <ctime>
#include <string>
#include <windows.h>
using namespace std;

/*
 * printCalendar() – prints the calendar for the given month/year.
 * Uses Zeller's congruence to find the starting weekday.
 */
void printCalendar(int month, int year){
    const char* months[]={"","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    int days[]={0,31,28,31,30,31,30,31,31,30,31,30,31};
    if(year%4==0&&(year%100!=0||year%400==0)) days[2]=29;
    // Zeller's congruence for first weekday (0=Sun)
    int m=month,y=year;
    if(m<3){m+=12;y--;}
    int k=y%100,j=y/100;
    int dow=(1+13*(m+1)/5+k+k/4+j/4+5*j)%7;
    dow=(dow+6)%7; // 0=Mon
    cout<<"\033[36m\n  === "<<months[month]<<" "<<year<<" ===\n"
        <<"  Mo  Tu  We  Th  Fr  Sa  Su\n\033[0m";
    int col=dow;
    for(int i=0;i<col;i++) cout<<"   ";
    for(int d=1;d<=days[month];d++){
        time_t t=time(nullptr); tm tm2{}; localtime_s(&tm2,&t);
        if(d==tm2.tm_mday&&month==tm2.tm_mon+1&&year==tm2.tm_year+1900)
            cout<<"\033[32m"<<(d<10?" ":"")<<d<<"\033[0m ";
        else cout<<(d<10?" ":"")<<d<<" ";
        col++;
        if(col%7==0) cout<<"\n";
    }
    cout<<"\n";
}

int main(){
    HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode=0; GetConsoleMode(h,&mode);
    SetConsoleMode(h,mode|ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleTitle("Ultron OS – Calendar [auto]");
    time_t t=time(nullptr); tm tm2{}; localtime_s(&tm2,&t);
cout << "\033[1;36m\n";
cout << "====================================================\n";
cout << "              ULTRON OS :: CALENDAR                 \n";
cout << "====================================================\n";
cout << "\033[0m";
    printCalendar(tm2.tm_mon+1, tm2.tm_year+1900);
    cout<<"  Press Enter to close.\n"; string s; getline(cin,s);
    return 0;
}
