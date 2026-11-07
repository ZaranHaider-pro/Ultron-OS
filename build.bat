@echo off
title ULTRON OS – Build Script
color 0A

echo.
echo  ===============================================
echo   ULTRON OS
echo   Hello Mr Zaran
echo  ===============================================
echo.

:: -- Play ULTRON Startup Sound -------------------------
start "" powershell.exe -NoProfile -WindowStyle Hidden -Command "if (Test-Path '%~dp0sounds\startup.wav') { $p = New-Object System.Media.SoundPlayer; $p.SoundLocation = '%~dp0sounds\startup.wav'; $p.Load(); $p.PlaySync() }"

:: -- Check g++ ---------------------------------------------------
where g++ >nul 2>&1
if errorlevel 1 (
    color 0C
    echo  [ERROR] g++ not found in PATH.
    echo.
    echo  Fix: Open "MSYS2 MinGW 64-bit" from Start Menu and run:
    echo    pacman -S mingw-w64-x86_64-gcc
    echo  Then add  C:\msys64\mingw64\bin  to your Windows PATH.
    echo.
    pause
    exit /b 1
)

:: -- Create output folders ----------------------------------------
if not exist tasks   mkdir tasks
if not exist data    mkdir data
if not exist logs    mkdir logs

:: -- Shared flags -------------------------------------------------
set FLAGS=-std=c++20 -Wall -Wextra -I./src

:: -- GUI flags (extra libs for Win32 GUI) -------------------------
set LIBS=-lpsapi -lpthread
set GUI_LIBS=-lgdi32 -lcomctl32 -lcomdlg32 -lpsapi -lpthread

:: -- Compile task executables ------------------------------------
echo  [1/15] Compiling task_notepad...
g++ tasks\task_notepad.cpp     -o tasks\task_notepad.exe     %FLAGS% %LIBS%
if errorlevel 1 goto :err

echo  [2/15] Compiling task_calculator...
g++ tasks\task_calculator.cpp  -o tasks\task_calculator.exe  %FLAGS% %LIBS%
if errorlevel 1 goto :err

echo  [3/15] Compiling task_clock...
g++ tasks\task_clock.cpp       -o tasks\task_clock.exe       %FLAGS% %LIBS%
if errorlevel 1 goto :err

echo  [4/15] Compiling task_filemanager...
g++ tasks\task_filemanager.cpp -o tasks\task_filemanager.exe %FLAGS% %LIBS%
if errorlevel 1 goto :err

echo  [5/15] Compiling task_minesweeper...
g++ tasks\task_minesweeper.cpp -o tasks\task_minesweeper.exe %FLAGS% %LIBS%
if errorlevel 1 goto :err

echo  [6/15] Compiling task_music...
g++ tasks\task_music.cpp       -o tasks\task_music.exe       %FLAGS% %LIBS%
if errorlevel 1 goto :err

echo  [7/15] Compiling task_sysinfo...
g++ tasks\task_sysinfo.cpp     -o tasks\task_sysinfo.exe     %FLAGS% %LIBS%
if errorlevel 1 goto :err

echo  [8/15] Compiling task_snake...
g++ tasks\task_snake.cpp       -o tasks\task_snake.exe       %FLAGS% %LIBS%
if errorlevel 1 goto :err

echo  [9/15] Compiling task_texteditor...
g++ tasks\task_texteditor.cpp  -o tasks\task_texteditor.exe  %FLAGS% %LIBS%
if errorlevel 1 goto :err

echo  [10/15] Compiling task_print...
g++ tasks\task_print.cpp       -o tasks\task_print.exe       %FLAGS% %LIBS%
if errorlevel 1 goto :err

echo  [11/15] Compiling task_calendar...
g++ tasks\task_calendar.cpp    -o tasks\task_calendar.exe    %FLAGS% %LIBS%
if errorlevel 1 goto :err

echo  [12/15] Compiling task_typingtest...
g++ tasks\task_typingtest.cpp  -o tasks\task_typingtest.exe  %FLAGS% %LIBS%
if errorlevel 1 goto :err

echo  [13/15] Compiling task_todo...
g++ tasks\task_todo.cpp        -o tasks\task_todo.exe        %FLAGS% %LIBS%
if errorlevel 1 goto :err

echo  [14/15] Compiling task_downloader...
g++ tasks\task_downloader.cpp  -o tasks\task_downloader.exe  %FLAGS% %LIBS%
if errorlevel 1 goto :err

echo  [15/15] Compiling task_processviewer...
g++ tasks\task_processviewer.cpp -o tasks\task_processviewer.exe %FLAGS% %LIBS%
if errorlevel 1 goto :err

echo.
echo  [MAIN-GUI] Compiling ULTRON OS GUI (ultron_gui.exe)...
g++ main.cpp -o ultron_gui.exe %FLAGS% -mwindows %GUI_LIBS%
if errorlevel 1 (
    echo  [WARN] GUI build failed. Skipping GUI executable.
) else (
    echo  [OK] ultron_gui.exe compiled successfully.
)

echo  [MAIN-CLI] Compiling ULTRON OS CLI (ultron_cli.exe)...
g++ main.cpp -o ultron_cli.exe %FLAGS% -DNO_GUI %LIBS%
if errorlevel 1 goto :err
echo  [OK] ultron_cli.exe compiled successfully.

:: -- Success -----------------------------------------------------
echo.
echo  ===============================================
echo   BUILD SUCCESSFUL!
echo  ===============================================
echo.
:: -- Play Build Completion Sound --------------------
powershell.exe -NoProfile -WindowStyle Hidden -Command "$p = New-Object System.Media.SoundPlayer('%~dp0sounds\complete.wav'); $p.PlaySync()"

echo.
echo  Run the OS with:
echo    ultron.exe
echo.
choice /M "Launch ULTRON OS now?"
if errorlevel 2 goto :done
start launch.bat
goto :done

:err
echo.
color 0C
echo  ===============================================
echo   BUILD FAILED – see error above.
echo  ===============================================
echo.
:: -- Play Build Completion Sound --------------------
powershell.exe -NoProfile -WindowStyle Hidden -Command "$p = New-Object System.Media.SoundPlayer('%~dp0sounds\complete.wav'); $p.PlaySync()"
pause
exit /b 1

:done
echo.
pause
