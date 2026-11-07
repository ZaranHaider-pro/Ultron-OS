# ULTRON OS – Operating System Simulator

ULTRON OS is a C++20 operating-system simulator with graphical and command-line interfaces. It provides an interactive way to explore operating-system concepts such as process management, CPU scheduling, memory allocation, resource management, authentication, and file-system operations.

> **Project type:** University project  
> **Platform:** Windows  
> **Language:** C++20

## Features

- **GUI and CLI modes:** Choose between a Windows graphical interface and a command-line shell.
- **Process management:** Create and terminate processes, inspect process control blocks (PCBs), interrupt and resume processes, minimize and restore processes, and simulate context switches.
- **CPU scheduling:** Run First-Come, First-Served (FCFS) and Round Robin scheduling, and inspect multi-level ready queues.
- **Memory management:** Display the memory map and allocate/free memory using First Fit and Best Fit strategies.
- **Resource management:** Acquire and release resources, inspect resource allocation, and run deadlock detection.
- **Inter-process communication:** Send and read messages between simulated processes.
- **System-call simulation:** Includes simulated `fork`, `exit`, `wait`, and `getpid` calls.
- **File-system operations:** Navigate directories and create, read, write, append, copy, move, and delete files.
- **User authentication:** Register, log in, log out, change passwords, delete users, and list users through the authentication manager.
- **Logging:** Records timestamped system events in `logs/system.log`.
- **Sound effects:** Plays a startup sound and a build-completion sound when the corresponding WAV files are present.
- **Built-in applications:** Includes utilities and games such as Notepad, Calculator, Clock, File Manager, Minesweeper, Music Player, System Information, Snake, Text Editor, Print, Calendar, Typing Test, To-Do, Downloader, and Process Viewer.

## Technology and Concepts

- C++20
- Windows API / Win32 GUI
- Multithreading and synchronization primitives from the C++ standard library
- Process Control Blocks (PCB)
- FCFS and Round Robin scheduling
- Multi-level ready queues
- First Fit and Best Fit memory allocation
- Resource allocation and deadlock detection
- File-system simulation
- Authentication and event logging

## Repository Structure

```text
ULTRON-OS/
├── build.bat                 # Builds task applications and GUI/CLI executables
├── launch.bat                # Opens the GUI/CLI selection menu
├── main.cpp                  # Application entry point
├── src/                      # Core simulator components
│   ├── auth.h
│   ├── filesystem.h
│   ├── gui.h
│   ├── logger.h
│   ├── memory_manager.h
│   ├── process_manager.h
│   ├── scheduler.h
│   ├── shell.h
│   └── ultron.h
├── tasks/                    # Individual built-in task applications
├── sounds/                   # Startup and build-completion WAV files
├── screenshots/              # Application screenshots
├── data/                     # Runtime data directory
└── logs/                     # Runtime log directory
```

## Requirements

- Windows 10 or later recommended.
- MSYS2 MinGW 64-bit toolchain.
- GCC with C++20 support (`g++`).
- Windows SDK libraries available through the MinGW toolchain for the GUI build.

The build script uses these libraries: `gdi32`, `comctl32`, `comdlg32`, `psapi`, and `pthread`.

## Build Instructions

1. Install [MSYS2](https://www.msys2.org/).
2. Open the **MSYS2 MinGW 64-bit** terminal and install the compiler:

   ```bash
   pacman -S --needed mingw-w64-x86_64-gcc
   ```

3. Add the MinGW compiler directory (usually `C:\msys64\mingw64\bin`) to the Windows `PATH` environment variable.
4. Open Command Prompt in the repository's root directory.
5. Run:

   ```bat
   build.bat
   ```

The script compiles the 15 task executables, attempts to build the GUI executable (`ultron_gui.exe`), and builds the CLI executable (`ultron_cli.exe`). A GUI compilation failure is reported as a warning by the script; the CLI build is required for the build script to complete successfully.

## Run Instructions

After a successful build, run:

```bat
launch.bat
```

Select one of the available modes:

- `1` – GUI Mode (`ultron_gui.exe`)
- `2` – CLI Mode (`ultron_cli.exe`)
- `3` – Exit

You can also run the GUI or CLI executable directly from the project directory.

## CLI Overview

The command-line shell provides commands for authentication, process management, scheduling, memory allocation, resource management, simulated system calls, inter-process messaging, file-system operations, and system logs.

Type `help` after launching the CLI to display the available commands and their usage.

## Screenshots

Screenshots are available in the [`screenshots/`](screenshots/) directory:

- [CLI](screenshots/CLI.png)
- [Compilation](screenshots/Compilation.png)
- [GUI](screenshots/GUI.png)
- [Login](screenshots/Login.png)

## Runtime Files

The application creates or uses `data/` and `logs/` directories while running. The logger writes session entries to `logs/system.log`. The build script also uses the files in `sounds/` for startup and completion audio.

## Academic Project

Developed as a university group project for the Software Engineering degree program at FAST-NUCES.

**Contributors:** Add the names of the project team members here.  
**My contribution:** Add the components and tasks you personally implemented here.

