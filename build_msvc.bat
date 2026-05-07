@echo off
REM Build DeepSeekBalancePlugin.dll using MSVC Build Tools
REM
REM Prerequisites:
REM   1. Install Visual Studio 2022 Build Tools (free):
REM      https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
REM      Select: "Desktop development with C++" workload
REM      Or install only: MSVC v143 - VS 2022 C++ x64/x86 build tools + Windows 10/11 SDK
REM
REM   2. Run this script from a "Developer Command Prompt for VS 2022"
REM      OR run: "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
REM      before running this script.

setlocal enabledelayedexpansion

REM Default architecture: x64 (change to x86 for 32-bit)
if "%1"=="" (
    set ARCH=x64
) else (
    set ARCH=%1
)

REM Source files
set SRCS=dllmain.cpp DataManager.cpp DeepSeekBalanceItem.cpp DeepSeekBalancePlugin.cpp OptionsDlg.cpp

REM Compiler flags
set CFLAGS=/O2 /std:c++17 /utf-8 /D"_USRDLL" /D"WIN32" /D"_WINDOWS" /D"_WIN32_WINNT=0x0600" /DWIN32_LEAN_AND_MEAN /W3 /MD /LD

REM Linker libraries
set LIBS=winhttp.lib comctl32.lib kernel32.lib user32.lib

echo === Building DeepSeek Balance Plugin for %ARCH% ===

REM Compile resource file
echo Compiling resources...
rc /fo DeepSeekBalancePlugin.res DeepSeekBalancePlugin.rc
if errorlevel 1 (
    echo ERROR: Resource compilation failed
    exit /b 1
)

REM Compile and link
echo Compiling source files and linking...
cl %SRCS% DeepSeekBalancePlugin.res %CFLAGS% /link /DLL /OUT:DeepSeekBalancePlugin.dll %LIBS%
if errorlevel 1 (
    echo ERROR: Build failed
    exit /b 1
)

echo.
echo === Build successful! ===
echo Output: DeepSeekBalancePlugin.dll
echo.
echo Install: Copy DeepSeekBalancePlugin.dll to TrafficMonitor\plugins\ directory
