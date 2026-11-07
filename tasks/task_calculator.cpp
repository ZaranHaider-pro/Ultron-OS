/*
 * ============================================================
 * task_calculator.cpp  –  Task 2: Calculator
 * Evaluates arithmetic expressions: <num> <op> <num>
 * Ops: + - * / %   |  Type 'close' to exit.
 * INTERACTIVE task type.
 * ============================================================
 */
#include <iostream>
#include <sstream>
#include <cmath>
#include <string>
#include <windows.h>
using namespace std;

/*
 * evaluate() – parses a simple two-operand expression
 * and returns the result. Returns false on bad input.
 */
bool evaluate(const string& expr, double& result) {
    // Support both "5 + 3" and "5+3" formats
    string e = expr;
    for (char& c : e) if (c=='+' || c=='-' || c=='*' || c=='/' || c=='%') {
        if (c!='-' || (&c!=&e[0] && isdigit((unsigned char)*(&c-1)))) {
            // pad operator with spaces for consistent parsing
            size_t pos = &c - &e[0];
            e = e.substr(0,pos) + " " + c + " " + e.substr(pos+1);
            break;
        }
    }
    istringstream ss(e);
    double a, b; char op;
    if (!(ss >> a >> op >> b)) return false;
    switch (op) {
        case '+': result = a + b; return true;
        case '-': result = a - b; return true;
        case '*': result = a * b; return true;
        case '/':
            if (b == 0) { cout << "\033[31m  Error: Division by zero.\033[0m\n"; return false; }
            result = a / b; return true;
        case '%': result = fmod(a, b); return true;
        default:  cout << "\033[31m  Unknown operator: " << op << "\033[0m\n"; return false;
    }
}

int main() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0; GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleTitle("Ultron OS – Calculator");
cout << "\033[1;36m\n";
cout << " ##+   ##+##+  ########+######+  ######+ ###+   ##+\n";
cout << " ##|   ##|##|  +==##+==+##+==##+##+===##+####+  ##|\n";
cout << " ##|   ##|##|     ##|   ######++##|   ##|##+##+ ##|\n";
cout << " ##|   ##|##|     ##|   ##+==##+##|   ##|##|+##+##|\n";
cout << " +######++#######+##|   ##|  ##|+######++##| +####|\n";
cout << "  +=====+ +======++=+   +=+  +=+ +=====+ +=+  +===+\n";
cout << "====================================================\n";
cout << "              ULTRON OS :: CALCULATOR               \n";
cout << "====================================================\n";
cout << "\033[0m";
    cout << "  Usage: <number> <op> <number>\n"
         << "  Ops  : + - * / %\n"
         << "  Type 'close' to exit.\n\n";

    string line;
    while (true) {
        cout << "  calc> ";
        if (!getline(cin, line) || line == "close" || line == "exit") break;
        double res = 0;
        if (evaluate(line, res))
            cout << "\033[32m  = " << res << "\033[0m\n";
    }
    cout << "\033[33m  Calculator closed.\033[0m\n";
    Sleep(1000);
    return 0;
}
