// ===================================================================
//  ULTRON OS  –  Win32 GUI Shell
//  gui.h  –  Graphical Desktop Interface
//
//  Replaces the terminal Shell with a Win32 windowed GUI.
//  Features:
//   • Boot splash screen with animated loading bar
//   • Login dialog (user/password)
//   • Desktop with taskbar, system-resource bar, clock
//   • App grid (15 task icons) – double-click to launch
//   • Running-processes panel (minimize / kill)
//   • Memory map visualiser (live bar chart)
//   • Kernel-mode toggle button
//   • Shutdown with confirmation dialog
//
//  Build extra flags needed:  -lgdi32 -lcomctl32 -lcomdlg32
//  (already included in updated build.bat)
// ===================================================================
#pragma once
#ifdef _WIN32

#include "ultron.h"
#include "logger.h"
#include "auth.h"
#include "process_manager.h"
#include "memory_manager.h"
#include "filesystem.h"
#include "scheduler.h"

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>

// -- Control IDs ----------------------------------------------------
#define ID_TIMER_CLOCK      101
#define ID_TIMER_PROCUPDATE 102
#define ID_TIMER_BOOT       103
#define ID_BTN_SHUTDOWN     201
#define ID_BTN_KMODE        202
#define ID_BTN_KILL         203
#define ID_BTN_MINIMIZE_P   204
#define ID_BTN_RESTORE_P    205
#define ID_BTN_MEMMAP       206
#define ID_BTN_LOGIN        207
#define ID_BTN_LOGOUT       208
#define ID_LIST_PROCS       301
#define ID_EDIT_LOG         302
#define ID_LABEL_RAM        303
#define ID_LABEL_HDD        304
#define ID_LABEL_CORES      305
#define ID_LABEL_USER       306
#define ID_LABEL_CLOCK      307
#define ID_PROGRESS_RAM     308
#define ID_PROGRESS_HDD     309

// App icon button IDs start at 1000
#define ID_APP_BASE         1000

// -- Color palette --------------------------------------------------
#define CLR_BG          RGB(15, 15, 25)
#define CLR_PANEL       RGB(25, 25, 40)
#define CLR_ACCENT      RGB(0, 180, 220)
#define CLR_ACCENT2     RGB(0, 120, 255)
#define CLR_TEXT        RGB(220, 230, 255)
#define CLR_TEXTDIM     RGB(120, 130, 160)
#define CLR_GREEN       RGB(0, 210, 100)
#define CLR_RED         RGB(220, 60, 60)
#define CLR_YELLOW      RGB(255, 200, 0)
#define CLR_TASKBAR     RGB(10, 10, 20)

// -- App descriptor -------------------------------------------------
struct AppInfo {
    const char* name;       // display name
    const char* cmd;        // launch key (matches shell launchTask)
    const char* icon;       // emoji / short symbol shown in button
    TaskType    type;
    QueueLevel  ql;
    int ram, hdd, cores, prio, burst;
};

static const AppInfo APPS[] = {
    {"Notepad",        "notepad",       "[N]", TaskType::INTERACTIVE, QueueLevel::HIGH,    64,  50,1,3, 30},
    {"Calculator",     "calculator",    "[C]", TaskType::INTERACTIVE, QueueLevel::HIGH,    32,  10,1,2, 20},
    {"Clock",          "clock",         "[T]", TaskType::AUTO,        QueueLevel::LOW,     32,  10,1,8,999},
    {"File Manager",   "filemanager",   "[F]", TaskType::INTERACTIVE, QueueLevel::HIGH,    64, 100,1,3, 40},
    {"Minesweeper",    "minesweeper",   "[M]", TaskType::INTERACTIVE, QueueLevel::HIGH,   128,  20,2,4, 60},
    {"Music",          "music",         "[~]", TaskType::BACKGROUND,  QueueLevel::MEDIUM,  64,  50,1,6,120},
    {"Sys Info",       "sysinfo",       "[i]",  TaskType::AUTO,        QueueLevel::LOW,     32,  10,1,7, 15},
    {"Snake",          "snake",         "[S]", TaskType::INTERACTIVE, QueueLevel::HIGH,   128,  20,2,4, 60},
    {"Text Editor",    "texteditor",    "[E]",  TaskType::INTERACTIVE, QueueLevel::HIGH,    64,  50,1,3, 30},
    {"Print",          "print",         "[P]",  TaskType::BACKGROUND,  QueueLevel::MEDIUM,  32, 200,1,5, 80},
    {"Calendar",       "calendar",      "[D]", TaskType::AUTO,        QueueLevel::LOW,     32,  10,1,9, 20},
    {"Typing Test",    "typingtest",    "[K]",  TaskType::INTERACTIVE, QueueLevel::HIGH,    64,  20,1,3, 30},
    {"To-Do",          "todo",          "[V]", TaskType::INTERACTIVE, QueueLevel::MEDIUM,  32,  50,1,4, 25},
    {"Downloader",     "downloader",    "[>]",  TaskType::BACKGROUND,  QueueLevel::MEDIUM,  64, 500,1,5,100},
    {"Proc Viewer",    "processviewer", "[R]", TaskType::AUTO,        QueueLevel::LOW,     64,  20,1,7, 20},
};
static const int APP_COUNT = 15;

// -- Forward declarations --------------------------------------------
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SplashProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK LoginDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK MemMapDlgProc(HWND, UINT, WPARAM, LPARAM);

// -- Global GUI state (accessed from callbacks) ----------------------
struct GUIState {
    Logger*         logger  = nullptr;
    Scheduler*      sched   = nullptr;
    AuthManager*    auth    = nullptr;
    ProcessManager* pm      = nullptr;
    MemoryManager*  mm      = nullptr;
    FileSystem*     fs      = nullptr;

    HWND hwndMain    = nullptr;
    HWND hwndListP   = nullptr;   // process list
    HWND hwndLog     = nullptr;   // log edit
    HWND hwndRamProg = nullptr;
    HWND hwndHddProg = nullptr;
    HWND hwndClock   = nullptr;
    HWND hwndUser    = nullptr;
    HWND hwndKBtn    = nullptr;

    HFONT hFontTitle  = nullptr;
    HFONT hFontMono   = nullptr;
    HFONT hFontSmall  = nullptr;
    HFONT hFontIcon   = nullptr;

    HBRUSH hbrBG      = nullptr;
    HBRUSH hbrPanel   = nullptr;
    HBRUSH hbrAccent  = nullptr;
    HBRUSH hbrTaskbar = nullptr;

    // Boot splash
    int  bootProgress = 0;
    bool bootDone     = false;
    HWND hwndSplash   = nullptr;

    // Resource tracking
    int totalRAM   = 2048;
    int totalHDD   = 256*1024;
    int totalCores = 8;
    int usedRAM    = 0;
    int usedHDD    = 0;
    int usedCores  = 0;

    // Log buffer
    std::string logBuffer;
};

static GUIState g;  // single global GUI state

// -------------------------------------------------------------------
//  Utility: append to the log edit box
// -------------------------------------------------------------------
static void AppendLog(const std::string& msg) {
    if (!g.hwndLog) return;
    std::string line = msg + "\r\n";
    int len = GetWindowTextLengthA(g.hwndLog);
    SendMessageA(g.hwndLog, EM_SETSEL, len, len);
    SendMessageA(g.hwndLog, EM_REPLACESEL, FALSE, (LPARAM)line.c_str());
    // Keep last 200 lines
    int lines = (int)SendMessageA(g.hwndLog, EM_GETLINECOUNT, 0, 0);
    if (lines > 200) {
        int excess = lines - 200;
        int endPos = (int)SendMessageA(g.hwndLog, EM_LINEINDEX, excess, 0);
        SendMessageA(g.hwndLog, EM_SETSEL, 0, endPos);
        SendMessageA(g.hwndLog, EM_REPLACESEL, FALSE, (LPARAM)"");
    }
}

// -------------------------------------------------------------------
//  Update resource progress bars & labels
// -------------------------------------------------------------------
static void UpdateResourceBars(HWND hwnd) {
    // RAM
    char buf[128];
    int ramPct = (g.totalRAM > 0) ? (g.usedRAM * 100 / g.totalRAM) : 0;
    snprintf(buf, sizeof(buf), "RAM: %d / %d MB  (%d%%)", g.usedRAM, g.totalRAM, ramPct);
    SetDlgItemTextA(hwnd, ID_LABEL_RAM, buf);
    if (g.hwndRamProg) SendMessage(g.hwndRamProg, PBM_SETPOS, ramPct, 0);

    int hddMB  = g.usedHDD / 1024;
    int tHddMB = g.totalHDD / 1024;
    int hddPct = (g.totalHDD > 0) ? (g.usedHDD * 100 / g.totalHDD) : 0;
    snprintf(buf, sizeof(buf), "HDD: %d / %d GB  (%d%%)",
             hddMB/1024, tHddMB/1024, hddPct);
    SetDlgItemTextA(hwnd, ID_LABEL_HDD, buf);
    if (g.hwndHddProg) SendMessage(g.hwndHddProg, PBM_SETPOS, hddPct, 0);

    snprintf(buf, sizeof(buf), "Cores: %d / %d", g.usedCores, g.totalCores);
    SetDlgItemTextA(hwnd, ID_LABEL_CORES, buf);
}

// -------------------------------------------------------------------
//  Refresh process list
// -------------------------------------------------------------------
static void RefreshProcList() {
    if (!g.hwndListP || !g.pm) return;

    // Save currently selected PID so we can restore it after reset
    int selectedPid = -1;
    int curSel = (int)SendMessage(g.hwndListP, LB_GETCURSEL, 0, 0);
    if (curSel != LB_ERR) {
        char selBuf[256] = {0};
        SendMessageA(g.hwndListP, LB_GETTEXT, curSel, (LPARAM)selBuf);
        sscanf(selBuf + 1, "%d", &selectedPid);
    }

    SendMessage(g.hwndListP, LB_RESETCONTENT, 0, 0);

    auto procs = g.pm->getProcList();
    g.usedRAM   = 0;
    g.usedHDD   = 0;
    g.usedCores = 0;
    int restoreIdx = LB_ERR;
    int insertIdx  = 0;
    for (auto& p : procs) {
        if (p.state == ProcessState::TERMINATED) continue;
        g.usedRAM   += p.ramMB;
        g.usedHDD   += p.hddMB;
        g.usedCores += p.coresUsed;
        char line[256];
        snprintf(line, sizeof(line),
                 "[%3d] %-18s %-10s  RAM:%3dMB  PRI:%d",
                 p.pid,
                 p.name.substr(0,18).c_str(),
                 stateToStr(p.state).c_str(),
                 p.ramMB, p.priority);
        SendMessageA(g.hwndListP, LB_ADDSTRING, 0, (LPARAM)line);
        if (p.pid == selectedPid) restoreIdx = insertIdx;
        insertIdx++;
    }
    // Restore the previously selected item
    if (restoreIdx != LB_ERR)
        SendMessage(g.hwndListP, LB_SETCURSEL, restoreIdx, 0);
}

// -------------------------------------------------------------------
//  Launch a task (wraps pm->spawnTask + log)
// -------------------------------------------------------------------
static void LaunchApp(int idx) {
    if (idx < 0 || idx >= APP_COUNT) return;
    if (!g.auth->isLoggedIn()) {
        MessageBoxA(g.hwndMain,
                    "Please log in first.",
                    "Not Logged In", MB_OK | MB_ICONWARNING);
        return;
    }
    const AppInfo& a = APPS[idx];
    // Check RAM
    if (g.usedRAM + a.ram > g.totalRAM) {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "Not enough RAM to launch %s.\nRequired: %dMB  Available: %dMB",
                 a.name, a.ram, g.totalRAM - g.usedRAM);
        MessageBoxA(g.hwndMain, msg, "Resource Error", MB_OK | MB_ICONERROR);
        return;
    }
    std::string exePath = std::string("tasks\\task_") + a.cmd + ".exe";
    int pid = g.pm->spawnTask(
        std::string("task_") + a.cmd,
        a.type, a.ql,
        a.ram, a.hdd, a.cores, a.prio, a.burst,
        g.auth->getCurrentUser(),
        exePath
    );
    char logmsg[128];
    snprintf(logmsg, sizeof(logmsg),
             "[LAUNCH] %s → PID %d", a.name, pid);
    AppendLog(logmsg);
    g.logger->info("GUI", logmsg);
    RefreshProcList();
    UpdateResourceBars(g.hwndMain);
}

// -------------------------------------------------------------------
//  Get selected PID from process list
// -------------------------------------------------------------------
static int GetSelectedPID() {
    int sel = (int)SendMessage(g.hwndListP, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR) return -1;
    char buf[256];
    SendMessageA(g.hwndListP, LB_GETTEXT, sel, (LPARAM)buf);
    // Format: "[  3] ..."
    int pid = -1;
    sscanf(buf + 1, "%d", &pid);
    return pid;
}

// -------------------------------------------------------------------
//  Memory Map dialog
// -------------------------------------------------------------------
INT_PTR CALLBACK MemMapDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM /*lParam*/) {
    switch (msg) {
    case WM_INITDIALOG: {
        SetWindowTextA(hDlg, "ULTRON OS – Memory Map");
        // Build text representation
        std::ostringstream oss;
        if (g.mm) {
            auto blocks = g.mm->getBlocks();
            int total   = g.mm->totalSize();
            oss << "  Total Memory: " << total << " KB\n";
            oss << "  -----------------------------------------\n";
            for (auto& b : blocks) {
                std::string label = b.isFree ? "FREE" : "PID " + std::to_string(b.allocatedPid);
                int barLen = std::max(1, b.size * 40 / std::max(total,1));
                std::string bar(barLen, b.isFree ? '.' : '#');
                char line[160];
                snprintf(line, sizeof(line),
                         "  %-8s [%-40s]  start=%5d  size=%d KB\n",
                         label.c_str(), bar.c_str(), b.start, b.size);
                oss << line;
            }
        } else {
            oss << "  Memory manager not available.\n";
        }
        HWND hEdit = GetDlgItem(hDlg, 1001);
        SetWindowTextA(hEdit, oss.str().c_str());
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            EndDialog(hDlg, 0);
        return TRUE;
    case WM_PAINT: {
        // Draw live bar chart at bottom of dialog
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);
        RECT rc; GetClientRect(hDlg, &rc);
        int chartY = rc.bottom - 50;
        int chartH = 30;
        int chartW = rc.right - 40;
        // Draw background
        HBRUSH brBg = CreateSolidBrush(RGB(20,20,35));
        RECT chartRc = {20, chartY, 20+chartW, chartY+chartH};
        FillRect(hdc, &chartRc, brBg);
        DeleteObject(brBg);
        if (g.mm) {
            auto blocks = g.mm->getBlocks();
            int total   = g.mm->totalSize();
            int x = 20;
            for (auto& b : blocks) {
                int w = std::max(1, b.size * chartW / std::max(total,1));
                COLORREF clr = b.isFree ? RGB(0,180,100) : RGB(220,100,0);
                HBRUSH br = CreateSolidBrush(clr);
                RECT r = {x, chartY, x+w, chartY+chartH};
                FillRect(hdc, &r, br);
                DeleteObject(br);
                x += w;
            }
        }
        EndPaint(hDlg, &ps);
        return 0;
    }
    }
    return FALSE;
}

// -------------------------------------------------------------------
//  Login dialog
// -------------------------------------------------------------------
INT_PTR CALLBACK LoginDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM /*lParam*/) {
    switch (msg) {
    case WM_INITDIALOG:
        SetDlgItemTextA(hDlg, 1001, "root");
        SetDlgItemTextA(hDlg, 1002, "root123");
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            char user[64], pass[64];
            GetDlgItemTextA(hDlg, 1001, user, sizeof(user));
            GetDlgItemTextA(hDlg, 1002, pass, sizeof(pass));
            if (g.auth->login(user, pass)) {
                char welcome[128];
                snprintf(welcome, sizeof(welcome),
                         "[AUTH] Welcome, %s!", user);
                AppendLog(welcome);
                // Update user label
                if (g.hwndUser)
                    SetWindowTextA(g.hwndUser, user);
                EndDialog(hDlg, IDOK);
            } else {
                MessageBoxA(hDlg, "Invalid username or password.",
                            "Login Failed", MB_OK | MB_ICONERROR);
            }
        } else if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
        }
        return TRUE;
    }
    return FALSE;
}

// -------------------------------------------------------------------
//  Draw rounded rectangle helper
// -------------------------------------------------------------------
static void DrawRoundRect(HDC hdc, int x, int y, int w, int h,
                           COLORREF fill, COLORREF border, int radius=8) {
    HBRUSH br  = CreateSolidBrush(fill);
    HPEN   pen = CreatePen(PS_SOLID, 1, border);
    HBRUSH oldBr  = (HBRUSH)SelectObject(hdc, br);
    HPEN   oldPen = (HPEN)SelectObject(hdc, pen);
    RoundRect(hdc, x, y, x+w, y+h, radius, radius);
    SelectObject(hdc, oldBr);
    SelectObject(hdc, oldPen);
    DeleteObject(br);
    DeleteObject(pen);
}

// -------------------------------------------------------------------
//  Splash screen proc (boot animation)
// -------------------------------------------------------------------
LRESULT CALLBACK SplashProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        SetTimer(hwnd, ID_TIMER_BOOT, 60, nullptr);
        return 0;
    }
    case WM_TIMER: {
        if (wParam == ID_TIMER_BOOT) {
            g.bootProgress += 2;
            InvalidateRect(hwnd, nullptr, TRUE);
            if (g.bootProgress >= 100) {
                KillTimer(hwnd, ID_TIMER_BOOT);
                g.bootDone = true;
                DestroyWindow(hwnd);
            }
        }
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);
        int W = rc.right, H = rc.bottom;

        // Background
        HBRUSH brBg = CreateSolidBrush(CLR_BG);
        FillRect(hdc, &rc, brBg);
        DeleteObject(brBg);

        // Glowing circle background effect
        for (int r = 200; r > 0; r -= 10) {
            int alpha = 255 - r;
            HBRUSH br = CreateSolidBrush(RGB(0, alpha/4, alpha/2));
            Ellipse(hdc, W/2-r, H/2-r-40, W/2+r, H/2+r-40);
            DeleteObject(br);
        }

        // Title
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, CLR_ACCENT);
        SelectObject(hdc, g.hFontTitle);
        const char* title = "ULTRON OS";
        SIZE sz;
        GetTextExtentPoint32A(hdc, title, (int)strlen(title), &sz);
        TextOutA(hdc, (W - sz.cx)/2, H/2 - 80, title, (int)strlen(title));

        // Subtitle
        SetTextColor(hdc, CLR_TEXTDIM);
        SelectObject(hdc, g.hFontSmall);
        const char* sub = "Operating System Simulator  v2.0";
        GetTextExtentPoint32A(hdc, sub, (int)strlen(sub), &sz);
        TextOutA(hdc, (W - sz.cx)/2, H/2 - 30, sub, (int)strlen(sub));

        // Owner
        const char* grp = "24F-3032  Zaran Haider";
        GetTextExtentPoint32A(hdc, grp, (int)strlen(grp), &sz);
        TextOutA(hdc, (W - sz.cx)/2, H/2 + 5, grp, (int)strlen(grp));

        // Progress bar track
        int bx = W/2 - 200, by = H/2 + 55, bw = 400, bh = 16;
        DrawRoundRect(hdc, bx-2, by-2, bw+4, bh+4, RGB(30,30,50), CLR_ACCENT, 10);

        // Progress bar fill
        int fillW = (int)(bw * g.bootProgress / 100.0);
        if (fillW > 0)
            DrawRoundRect(hdc, bx, by, fillW, bh, CLR_ACCENT, CLR_ACCENT, 8);

        // Percentage
        char pct[16];
        snprintf(pct, sizeof(pct), "Loading... %d%%", g.bootProgress);
        GetTextExtentPoint32A(hdc, pct, (int)strlen(pct), &sz);
        SetTextColor(hdc, CLR_TEXT);
        TextOutA(hdc, (W - sz.cx)/2, by + 25, pct, (int)strlen(pct));

        // Boot message
        const char* msgs[] = {
            "Initialising kernel...",
            "Loading drivers...",
            "Starting scheduler...",
            "Mounting file system...",
            "Starting services...",
            "Loading desktop...",
        };
        int mi = std::min((int)(g.bootProgress / 17), 5);
        SetTextColor(hdc, CLR_TEXTDIM);
        GetTextExtentPoint32A(hdc, msgs[mi], (int)strlen(msgs[mi]), &sz);
        TextOutA(hdc, (W - sz.cx)/2, by + 50, msgs[mi], (int)strlen(msgs[mi]));

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        // Show main window after splash
        if (g.hwndMain) ShowWindow(g.hwndMain, SW_SHOW);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// -------------------------------------------------------------------
//  Main window proc
// -------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    // -- Draw background -----------------------------------------
    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rc; GetClientRect(hwnd, &rc);
        HBRUSH br = CreateSolidBrush(CLR_BG);
        FillRect(hdc, &rc, br);
        DeleteObject(br);
        return 1;
    }

    // -- Custom paint for panels & labels ------------------------
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);
        int W = rc.right;

        // Taskbar at bottom
        RECT tbRc = {0, rc.bottom-40, W, rc.bottom};
        HBRUSH brTb = CreateSolidBrush(CLR_TASKBAR);
        FillRect(hdc, &tbRc, brTb);
        DeleteObject(brTb);

        // Taskbar separator line
        HPEN pen = CreatePen(PS_SOLID, 1, CLR_ACCENT);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        MoveToEx(hdc, 0, rc.bottom-40, nullptr);
        LineTo(hdc,   W, rc.bottom-40);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);

        // Side panel background (right side, x=860..W)
        RECT sideRc = {860, 0, W, rc.bottom-40};
        HBRUSH brSide = CreateSolidBrush(CLR_PANEL);
        FillRect(hdc, &sideRc, brSide);
        DeleteObject(brSide);

        // Side panel header
        SetBkMode(hdc, TRANSPARENT);
        SelectObject(hdc, g.hFontSmall);
        SetTextColor(hdc, CLR_ACCENT);
        RECT hdrRc = {870, 5, W-10, 25};
        DrawTextA(hdc, "SYSTEM RESOURCES", -1, &hdrRc, DT_LEFT);

        RECT p2Rc = {870, 200, W-10, 220};
        DrawTextA(hdc, "RUNNING PROCESSES", -1, &p2Rc, DT_LEFT);

        RECT p3Rc = {870, 420, W-10, 440};
        DrawTextA(hdc, "SYSTEM LOG", -1, &p3Rc, DT_LEFT);

        // Separator lines in panel
        pen = CreatePen(PS_SOLID, 1, CLR_ACCENT);
        oldPen = (HPEN)SelectObject(hdc, pen);
        for (int y : {25, 215, 425, 195, 415}) {
            MoveToEx(hdc, 865, y, nullptr);
            LineTo(hdc,   W-5, y);
        }
        SelectObject(hdc, oldPen);
        DeleteObject(pen);

        // App grid section header
        SetTextColor(hdc, CLR_ACCENT);
        SelectObject(hdc, g.hFontSmall);
        RECT appsHdr = {10, 5, 850, 25};
        DrawTextA(hdc, "ULTRON OS  –  Applications", -1, &appsHdr, DT_LEFT);

        pen = CreatePen(PS_SOLID, 1, CLR_ACCENT);
        oldPen = (HPEN)SelectObject(hdc, pen);
        MoveToEx(hdc, 5, 25, nullptr);
        LineTo(hdc,  855, 25);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);

        EndPaint(hwnd, &ps);
        return 0;
    }

    // -- Colour child controls ------------------------------------
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, CLR_TEXT);
        int id = GetDlgCtrlID((HWND)lParam);
        if (id == ID_LABEL_RAM || id == ID_LABEL_HDD ||
            id == ID_LABEL_CORES || id == ID_LABEL_CLOCK) {
            SetTextColor(hdc, CLR_ACCENT);
        }
        if (id == ID_LABEL_USER) SetTextColor(hdc, CLR_GREEN);
        // Return panel brush for right-panel labels
        RECT r; GetWindowRect((HWND)lParam, &r);
        POINT pt = {r.left, r.top};
        ScreenToClient(hwnd, &pt);
        if (pt.x >= 860) return (LRESULT)g.hbrPanel;
        return (LRESULT)g.hbrBG;
    }
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX: {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, RGB(20,20,35));
        SetTextColor(hdc, CLR_TEXT);
        return (LRESULT)CreateSolidBrush(RGB(20,20,35));
    }

    // -- Timer ----------------------------------------------------
    case WM_TIMER: {
        if (wParam == ID_TIMER_CLOCK) {
            // Update clock
            SYSTEMTIME st;
            GetLocalTime(&st);
            char buf[32];
            snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
                     st.wHour, st.wMinute, st.wSecond);
            if (g.hwndClock) SetWindowTextA(g.hwndClock, buf);
        }
        if (wParam == ID_TIMER_PROCUPDATE) {
            RefreshProcList();
            UpdateResourceBars(hwnd);
        }
        return 0;
    }

    // -- Button / menu commands -----------------------------------
    case WM_COMMAND: {
        int id = LOWORD(wParam);

        // App icon buttons
        if (id >= ID_APP_BASE && id < ID_APP_BASE + APP_COUNT) {
            LaunchApp(id - ID_APP_BASE);
            return 0;
        }

        switch (id) {
        case ID_BTN_SHUTDOWN: {
            int r = MessageBoxA(hwnd,
                "Are you sure you want to shut down ULTRON OS?",
                "Shutdown", MB_YESNO | MB_ICONQUESTION);
            if (r == IDYES) {
                AppendLog("[SYSTEM] Shutdown initiated.");
                PostQuitMessage(0);
            }
            break;
        }
        case ID_BTN_KMODE: {
            bool k = !g.pm->isKernelMode();
            g.pm->setKernelMode(k);
            SetWindowTextA(g.hwndKBtn,
                           k ? "Kernel Mode: ON" : "Kernel Mode: OFF");
            char logm[64];
            snprintf(logm, sizeof(logm),
                     "[MODE] Switched to %s", k ? "KERNEL" : "USER");
            AppendLog(logm);
            InvalidateRect(hwnd, nullptr, FALSE);
            break;
        }
        case ID_BTN_KILL: {
            int pid = GetSelectedPID();
            if (pid < 0) {
                MessageBoxA(hwnd, "Select a process first.",
                            "Kill Process", MB_OK | MB_ICONWARNING);
                break;
            }
            if (!g.pm->isKernelMode()) {
                MessageBoxA(hwnd,
                    "Kernel mode required to kill processes.\nToggle Kernel Mode first.",
                    "Permission Denied", MB_OK | MB_ICONERROR);
                break;
            }
            char conf[128];
            snprintf(conf, sizeof(conf),
                     "Kill process PID %d?", pid);
            if (MessageBoxA(hwnd, conf, "Confirm Kill",
                            MB_YESNO | MB_ICONWARNING) == IDYES) {
                g.pm->terminateProcess(pid);
                char logm[64];
                snprintf(logm, sizeof(logm),
                         "[KILL] PID %d terminated.", pid);
                AppendLog(logm);
                RefreshProcList();
                UpdateResourceBars(hwnd);
            }
            break;
        }
        case ID_BTN_MINIMIZE_P: {
            int pid = GetSelectedPID();
            if (pid < 0) break;
            g.pm->minimize(pid);
            char logm[64];
            snprintf(logm, sizeof(logm), "[MIN] PID %d minimized.", pid);
            AppendLog(logm);
            RefreshProcList();
            break;
        }
        case ID_BTN_RESTORE_P: {
            int pid = GetSelectedPID();
            if (pid < 0) break;
            g.pm->restore(pid);
            char logm[64];
            snprintf(logm, sizeof(logm), "[RESTORE] PID %d restored.", pid);
            AppendLog(logm);
            RefreshProcList();
            break;
        }
        case ID_BTN_MEMMAP: {
            // Show memory map dialog
            HWND hDlg = CreateWindowExA(
                0, "ULTRON_MEMMAP", "Memory Map",
                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                200, 100, 700, 500,
                hwnd, nullptr,
                GetModuleHandle(nullptr), nullptr
            );
            (void)hDlg;
            break;
        }
        case ID_BTN_LOGIN: {
            // Show login dialog built inline
            HWND hDlg = CreateWindowExA(
                WS_EX_DLGMODALFRAME,
                "ULTRON_LOGIN", "Login to ULTRON OS",
                WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
                400, 250, 340, 200,
                hwnd, nullptr,
                GetModuleHandle(nullptr), nullptr
            );
            (void)hDlg;
            break;
        }
        case ID_BTN_LOGOUT:
            if (g.auth->isLoggedIn()) {
                g.auth->logout();
                if (g.hwndUser) SetWindowTextA(g.hwndUser, "guest");
                AppendLog("[AUTH] Logged out.");
            }
            break;
        }
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// -------------------------------------------------------------------
//  Memmap window proc (standalone)
// -------------------------------------------------------------------
LRESULT CALLBACK MemMapWndProc(HWND hwnd, UINT msg,
                                WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        // Text area
        CreateWindowExA(WS_EX_CLIENTEDGE,
            "EDIT", "",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
            ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
            10, 10, 665, 360,
            hwnd, (HMENU)1001,
            GetModuleHandle(nullptr), nullptr
        );
        // OK button
        CreateWindowA("BUTTON", "Close",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            300, 380, 80, 30,
            hwnd, (HMENU)IDOK,
            GetModuleHandle(nullptr), nullptr
        );
        // Fill text
        std::ostringstream oss;
        if (g.mm) {
            auto blocks = g.mm->getBlocks();
            int total   = g.mm->totalSize();
            oss << "Total Memory: " << total << " KB\r\n";
            oss << "---------------------------------------------\r\n";
            for (auto& b : blocks) {
                std::string lbl = b.isFree ? "FREE" : "PID " + std::to_string(b.allocatedPid);
                int barLen = std::max(1, b.size * 40 / std::max(total,1));
                std::string bar(barLen, b.isFree ? '.' : '#');
                char line[180];
                snprintf(line, sizeof(line),
                         "%-8s  [%-40s]  start=%5d  size=%d KB\r\n",
                         lbl.c_str(), bar.c_str(), b.start, b.size);
                oss << line;
            }
        }
        SetDlgItemTextA(hwnd, 1001, oss.str().c_str());
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);
        // Draw bar chart at bottom
        int chartY = 375, chartH = 25;
        int chartW = rc.right - 20;
        HBRUSH brBg = CreateSolidBrush(RGB(10,10,20));
        RECT chartRc = {10, chartY, 10+chartW, chartY+chartH};
        FillRect(hdc, &chartRc, brBg); DeleteObject(brBg);
        if (g.mm) {
            auto blocks = g.mm->getBlocks();
            int total   = g.mm->totalSize();
            int x = 10;
            for (auto& b : blocks) {
                int w = std::max(1, b.size * chartW / std::max(total,1));
                COLORREF clr = b.isFree ? RGB(0,180,100) : RGB(220,100,0);
                HBRUSH br = CreateSolidBrush(clr);
                RECT r = {x, chartY, x+w, chartY+chartH};
                FillRect(hdc, &r, br); DeleteObject(br);
                x += w;
            }
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// -------------------------------------------------------------------
//  Login window proc (standalone)
// -------------------------------------------------------------------
LRESULT CALLBACK LoginWndProc(HWND hwnd, UINT msg,
                               WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        // User label + edit
        CreateWindowA("STATIC", "Username:",
            WS_CHILD | WS_VISIBLE,
            20, 20, 100, 20, hwnd, nullptr,
            GetModuleHandle(nullptr), nullptr);
        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "root",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            130, 18, 170, 22, hwnd, (HMENU)1001,
            GetModuleHandle(nullptr), nullptr);
        // Pass label + edit
        CreateWindowA("STATIC", "Password:",
            WS_CHILD | WS_VISIBLE,
            20, 55, 100, 20, hwnd, nullptr,
            GetModuleHandle(nullptr), nullptr);
        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "root123",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD,
            130, 53, 170, 22, hwnd, (HMENU)1002,
            GetModuleHandle(nullptr), nullptr);
        // Buttons
        CreateWindowA("BUTTON", "Login",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            80, 100, 80, 28, hwnd, (HMENU)IDOK,
            GetModuleHandle(nullptr), nullptr);
        CreateWindowA("BUTTON", "Cancel",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            175, 100, 80, 28, hwnd, (HMENU)IDCANCEL,
            GetModuleHandle(nullptr), nullptr);
        return 0;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == IDOK) {
            char user[64], pass[64];
            GetDlgItemTextA(hwnd, 1001, user, sizeof(user));
            GetDlgItemTextA(hwnd, 1002, pass, sizeof(pass));
            if (g.auth && g.auth->login(user, pass)) {
                char welcome[128];
                snprintf(welcome, sizeof(welcome),
                         "[AUTH] Welcome, %s!", user);
                AppendLog(welcome);
                if (g.hwndUser) SetWindowTextA(g.hwndUser, user);
                DestroyWindow(hwnd);
            } else {
                MessageBoxA(hwnd, "Invalid credentials.",
                            "Login Failed", MB_OK | MB_ICONERROR);
            }
        } else if (LOWORD(wParam) == IDCANCEL) {
            DestroyWindow(hwnd);
        }
        return 0;
    }
    case WM_DESTROY: return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// -------------------------------------------------------------------
//  App icon button subclass (hover highlight)
// -------------------------------------------------------------------
static WNDPROC g_origBtnProc = nullptr;
LRESULT CALLBACK AppBtnSubclass(HWND hwnd, UINT msg,
                                  WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_MOUSEMOVE: {
        // Request repaint for hover effect
        TRACKMOUSEEVENT tme{sizeof(tme), TME_LEAVE, hwnd, 0};
        TrackMouseEvent(&tme);
        InvalidateRect(hwnd, nullptr, FALSE);
        break;
    }
    case WM_MOUSELEAVE:
        InvalidateRect(hwnd, nullptr, FALSE);
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);
        int idx = GetDlgCtrlID(hwnd) - ID_APP_BASE;

        // Check hover
        POINT cur; GetCursorPos(&cur); ScreenToClient(hwnd, &cur);
        bool hover = (cur.x >= 0 && cur.y >= 0 &&
                      cur.x < rc.right && cur.y < rc.bottom);

        COLORREF bg     = hover ? CLR_ACCENT2 : RGB(30,35,55);
        COLORREF border = hover ? CLR_ACCENT  : RGB(50,60,90);
        DrawRoundRect(hdc, 0, 0, rc.right, rc.bottom, bg, border, 10);

        // Icon text
        SetBkMode(hdc, TRANSPARENT);
        if (idx >= 0 && idx < APP_COUNT) {
            // Icon (emoji – may not render on all systems, falls back to text)
            SelectObject(hdc, g.hFontIcon);
            SetTextColor(hdc, hover ? RGB(255,255,255) : CLR_ACCENT);
            RECT iconRc = {0, 6, rc.right, rc.bottom/2+4};
            DrawTextA(hdc, APPS[idx].icon, -1, &iconRc,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            // Name
            SelectObject(hdc, g.hFontSmall);
            SetTextColor(hdc, hover ? RGB(255,255,255) : CLR_TEXT);
            RECT nameRc = {2, rc.bottom/2+4, rc.right-2, rc.bottom-2};
            DrawTextA(hdc, APPS[idx].name, -1, &nameRc,
                      DT_CENTER | DT_WORDBREAK);
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return CallWindowProc(g_origBtnProc, hwnd, msg, wParam, lParam);
}

// -------------------------------------------------------------------
//  Create all child controls
// -------------------------------------------------------------------
static void CreateControls(HWND hwnd) {
    HINSTANCE hInst = GetModuleHandle(nullptr);

    // -- App grid: 5 columns × 3 rows --------------------------
    //   x: 10..845, y: 30..580
    const int COLS = 5;
    const int BW = 158, BH = 90, GAPX = 8, GAPY = 10;
    int startX = 10, startY = 30;
    for (int i = 0; i < APP_COUNT; i++) {
        int col = i % COLS, row = i / COLS;
        int x   = startX + col*(BW+GAPX);
        int y   = startY + row*(BH+GAPY);
        HWND hBtn = CreateWindowA(
            "BUTTON", APPS[i].name,
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            x, y, BW, BH,
            hwnd, (HMENU)(UINT_PTR)(ID_APP_BASE + i),
            hInst, nullptr
        );
        // Subclass for custom paint
        if (!g_origBtnProc)
            g_origBtnProc = (WNDPROC)GetWindowLongPtr(hBtn, GWLP_WNDPROC);
        SetWindowLongPtr(hBtn, GWLP_WNDPROC, (LONG_PTR)AppBtnSubclass);
    }

    // -- Right panel: resource labels -------------------------
    int px = 870;

    CreateWindowA("STATIC", "RAM: – / – MB",
        WS_CHILD | WS_VISIBLE,
        px, 30, 250, 18, hwnd, (HMENU)ID_LABEL_RAM, hInst, nullptr);

    g.hwndRamProg = CreateWindowExA(0, PROGRESS_CLASS, nullptr,
        WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
        px, 52, 250, 12, hwnd, (HMENU)ID_PROGRESS_RAM, hInst, nullptr);
    SendMessage(g.hwndRamProg, PBM_SETRANGE, 0, MAKELPARAM(0,100));
    SendMessage(g.hwndRamProg, PBM_SETPOS, 0, 0);

    CreateWindowA("STATIC", "HDD: – / – GB",
        WS_CHILD | WS_VISIBLE,
        px, 72, 250, 18, hwnd, (HMENU)ID_LABEL_HDD, hInst, nullptr);

    g.hwndHddProg = CreateWindowExA(0, PROGRESS_CLASS, nullptr,
        WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
        px, 94, 250, 12, hwnd, (HMENU)ID_PROGRESS_HDD, hInst, nullptr);
    SendMessage(g.hwndHddProg, PBM_SETRANGE, 0, MAKELPARAM(0,100));
    SendMessage(g.hwndHddProg, PBM_SETPOS, 0, 0);

    CreateWindowA("STATIC", "Cores: 0 / 8",
        WS_CHILD | WS_VISIBLE,
        px, 114, 250, 18, hwnd, (HMENU)ID_LABEL_CORES, hInst, nullptr);

    // -- User info ---------------------------------------------
    CreateWindowA("STATIC", "User:",
        WS_CHILD | WS_VISIBLE,
        px, 145, 50, 18, hwnd, nullptr, hInst, nullptr);
    g.hwndUser = CreateWindowA("STATIC", "guest",
        WS_CHILD | WS_VISIBLE,
        px+52, 145, 180, 18, hwnd, (HMENU)ID_LABEL_USER, hInst, nullptr);

    // -- Login / Logout buttons --------------------------------
    CreateWindowA("BUTTON", "Login",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        px, 168, 75, 24, hwnd, (HMENU)ID_BTN_LOGIN, hInst, nullptr);
    CreateWindowA("BUTTON", "Logout",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        px+80, 168, 75, 24, hwnd, (HMENU)ID_BTN_LOGOUT, hInst, nullptr);

    // -- Process list -----------------------------------------
    g.hwndListP = CreateWindowExA(WS_EX_CLIENTEDGE,
        "LISTBOX", nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
        px, 220, 250, 185,
        hwnd, (HMENU)ID_LIST_PROCS, hInst, nullptr);
    SendMessage(g.hwndListP, WM_SETFONT, (WPARAM)g.hFontMono, TRUE);

    // Process control buttons
    CreateWindowA("BUTTON", "Kill",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        px, 408, 78, 24, hwnd, (HMENU)ID_BTN_KILL, hInst, nullptr);
    CreateWindowA("BUTTON", "Minimize",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        px+82, 408, 78, 24, hwnd, (HMENU)ID_BTN_MINIMIZE_P, hInst, nullptr);
    CreateWindowA("BUTTON", "Restore",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        px+164, 408, 78, 24, hwnd, (HMENU)ID_BTN_RESTORE_P, hInst, nullptr);

    // -- Log box ----------------------------------------------
    g.hwndLog = CreateWindowExA(WS_EX_CLIENTEDGE,
        "EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL |
        ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
        px, 440, 250, 160,
        hwnd, (HMENU)ID_EDIT_LOG, hInst, nullptr);
    SendMessage(g.hwndLog, WM_SETFONT, (WPARAM)g.hFontMono, TRUE);

    // -- Memory map button -------------------------------------
    CreateWindowA("BUTTON", "Memory Map",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        px, 607, 250, 24, hwnd, (HMENU)ID_BTN_MEMMAP, hInst, nullptr);

    // -- Kernel mode toggle ------------------------------------
    g.hwndKBtn = CreateWindowA("BUTTON", "Kernel Mode: OFF",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        px, 635, 250, 24, hwnd, (HMENU)ID_BTN_KMODE, hInst, nullptr);

    // -- Taskbar: clock + shutdown -----------------------------
    RECT rc; GetClientRect(hwnd, &rc);
    int tbY = rc.bottom - 36;

    // Clock in the CENTER of taskbar
    g.hwndClock = CreateWindowA("STATIC", "00:00:00",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        rc.right/2 - 55, tbY, 110, 28,
        hwnd, (HMENU)ID_LABEL_CLOCK, hInst, nullptr);
    SendMessage(g.hwndClock, WM_SETFONT, (WPARAM)g.hFontSmall, TRUE);

    // OS name on the left of taskbar
    CreateWindowA("STATIC", "ULTRON OS",
        WS_CHILD | WS_VISIBLE,
        10, tbY, 120, 28, hwnd, nullptr, hInst, nullptr);

    // Shutdown button on the far right of taskbar
    CreateWindowA("BUTTON", "Shutdown",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        rc.right - 130, tbY+2, 120, 26,
        hwnd, (HMENU)ID_BTN_SHUTDOWN, hInst, nullptr);
}

// -------------------------------------------------------------------
//  Main GUI entry point – call this from main()
// -------------------------------------------------------------------
class GUIShell {
    Logger         logger;
    Scheduler      sched;
    AuthManager    auth;
    ProcessManager pm;
    MemoryManager  mm;
    FileSystem     fs;

public:
    GUIShell()
        : logger(false), sched(logger), auth(logger),
          pm(logger, sched), mm(65536, logger), fs(logger) {
        g.logger = &logger;
        g.sched  = &sched;
        g.auth   = &auth;
        g.pm     = &pm;
        g.mm     = &mm;
        g.fs     = &fs;
    }

    int run(HINSTANCE hInst, int /*nCmdShow*/) {
        // Enable ANSI (not needed for GUI but keep for log file)
        system("chcp 65001 > nul");
        InitCommonControls();

        // -- Create fonts --------------------------------------
        g.hFontTitle = CreateFontA(
            42, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH, "Segoe UI"
        );
        g.hFontMono = CreateFontA(
            13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            FIXED_PITCH, "Consolas"
        );
        g.hFontSmall = CreateFontA(
            14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH, "Segoe UI"
        );
        g.hFontIcon = CreateFontA(
            22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH, "Courier New"
        );

        // -- Create brushes ------------------------------------
        g.hbrBG      = CreateSolidBrush(CLR_BG);
        g.hbrPanel   = CreateSolidBrush(CLR_PANEL);
        g.hbrAccent  = CreateSolidBrush(CLR_ACCENT);
        g.hbrTaskbar = CreateSolidBrush(CLR_TASKBAR);

        // -- Register window classes ---------------------------
        WNDCLASSEXA wc{};
        wc.cbSize        = sizeof(wc);
        wc.lpfnWndProc   = WndProc;
        wc.hInstance     = hInst;
        wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = g.hbrBG;
        wc.lpszClassName = "ULTRON_MAIN";
        wc.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
        RegisterClassExA(&wc);

        WNDCLASSEXA swc{};
        swc.cbSize        = sizeof(swc);
        swc.lpfnWndProc   = SplashProc;
        swc.hInstance     = hInst;
        swc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
        swc.hbrBackground = CreateSolidBrush(CLR_BG);
        swc.lpszClassName = "ULTRON_SPLASH";
        RegisterClassExA(&swc);

        WNDCLASSEXA mwc{};
        mwc.cbSize        = sizeof(mwc);
        mwc.lpfnWndProc   = MemMapWndProc;
        mwc.hInstance     = hInst;
        mwc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
        mwc.hbrBackground = CreateSolidBrush(CLR_BG);
        mwc.lpszClassName = "ULTRON_MEMMAP";
        RegisterClassExA(&mwc);

        WNDCLASSEXA lwc{};
        lwc.cbSize        = sizeof(lwc);
        lwc.lpfnWndProc   = LoginWndProc;
        lwc.hInstance     = hInst;
        lwc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
        lwc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
        lwc.lpszClassName = "ULTRON_LOGIN";
        RegisterClassExA(&lwc);

        // -- Main window (hidden until splash done) -----------
        g.hwndMain = CreateWindowExA(
            0, "ULTRON_MAIN", "ULTRON OS  –  Operating System Simulator",
            WS_OVERLAPPEDWINDOW,
            50, 30, 1140, 680,
            nullptr, nullptr, hInst, nullptr
        );

        CreateControls(g.hwndMain);
        UpdateWindow(g.hwndMain);

        // -- Splash screen --------------------------------------
        g.hwndSplash = CreateWindowExA(
            WS_EX_TOPMOST,
            "ULTRON_SPLASH", "ULTRON OS",
            WS_POPUP | WS_VISIBLE,
            150, 100, 800, 400,
            nullptr, nullptr, hInst, nullptr
        );
        UpdateWindow(g.hwndSplash);

        // -- Timers --------------------------------------------
        SetTimer(g.hwndMain, ID_TIMER_CLOCK,      1000, nullptr);
        SetTimer(g.hwndMain, ID_TIMER_PROCUPDATE, 2000, nullptr);

        // Auto-start background processes after boot
        pm.spawnTask("task_clock",    TaskType::AUTO, QueueLevel::LOW,
                     32, 10, 1, 8, 999, "system", "tasks\\task_clock.exe");
        pm.spawnTask("task_calendar", TaskType::AUTO, QueueLevel::LOW,
                     32, 10, 1, 9,  20, "system", "tasks\\task_calendar.exe");
        AppendLog("[BOOT] Clock and Calendar auto-started.");
        AppendLog("[BOOT] ULTRON OS ready.");

        // -- Message loop --------------------------------------
        MSG msgVal;
        while (GetMessage(&msgVal, nullptr, 0, 0)) {
            TranslateMessage(&msgVal);
            DispatchMessage(&msgVal);
        }

        // Cleanup
        DeleteObject(g.hFontTitle);
        DeleteObject(g.hFontMono);
        DeleteObject(g.hFontSmall);
        DeleteObject(g.hFontIcon);
        DeleteObject(g.hbrBG);
        DeleteObject(g.hbrPanel);
        DeleteObject(g.hbrAccent);
        DeleteObject(g.hbrTaskbar);

        return (int)msgVal.wParam;
    }
};

#endif // _WIN32
