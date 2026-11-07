// ===================================================================
//  ULTRON OS  –  Operating System Simulator
//
//  Build modes:
//    GUI mode (default):
//        g++ main.cpp -o ultron.exe -std=c++20 -lgdi32 -lcomctl32
//            -lcomdlg32 -lpsapi -lpthread -mwindows -Wall -I./src
//
//    Terminal mode:
//        g++ main.cpp -o ultron.exe -std=c++20 -lpsapi -lpthread
//            -DNO_GUI -Wall -I./src
// ===================================================================

#ifdef _WIN32
  #include <windows.h>
#endif

#ifndef NO_GUI
  #define USE_GUI
#endif

#ifdef USE_GUI
  #include "src/gui.h"

  int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
      GUIShell gui;
      return gui.run(hInst, nCmdShow);
  }

  int main() {
      HINSTANCE hInst = GetModuleHandle(nullptr);
      GUIShell gui;
      return gui.run(hInst, SW_SHOW);
  }

#else
  #include "src/shell.h"

  int main() {
      Shell shell;
      shell.run();
      return 0;
  }
#endif
