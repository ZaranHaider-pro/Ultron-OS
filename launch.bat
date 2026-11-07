@echo off
title ULTRON OS - Launcher
color 0A

:menu
cls
echo.
echo  +------------------------------------------+
echo  ^|         ULTRON OS  -  Launcher           ^|
echo  +------------------------------------------+
echo.
echo  Select launch mode:
echo.
echo    [1]  GUI Mode    (Graphical Desktop)
echo    [2]  CLI Mode    (Terminal / Command Line)
echo    [3]  Exit
echo.
set "choice="
set /p choice="  Enter choice (1/2/3): "

if "%choice%"=="1" goto :gui
if "%choice%"=="2" goto :cli
if "%choice%"=="3" goto :bye

echo.
echo  Invalid choice. Please enter 1, 2 or 3.
timeout /t 2 >nul
goto :menu

:gui
echo.
echo  Starting ULTRON OS in GUI mode...
timeout /t 1 >nul
start "" ultron_gui.exe
goto :bye

:cli
echo.
echo  Starting ULTRON OS in CLI mode...
timeout /t 1 >nul
ultron_cli.exe
goto :bye

:bye
exit
