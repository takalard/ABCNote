@echo off
REM This script configures ABCNote Release with the local MSVC, CMake, Ninja, and Qt toolchain.

REM SCRIPT_DIR is the absolute directory that contains this script.
set "SCRIPT_DIR=%~dp0"

REM PROJECT_DIR is the repository root, one level above the tools directory.
set "PROJECT_DIR=%SCRIPT_DIR%.."

REM VS_VCVARS is the Visual Studio environment setup script for x64 compilation.
set "VS_VCVARS=D:\Program\dev\MicrosoftVisualStudio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

REM CMAKE_EXE is the CMake executable installed under D:\Program\dev.
set "CMAKE_EXE=D:\Program\dev\CMake\bin\cmake.exe"

REM QT_PREFIX is the Qt 6 MSVC package root installed by aqtinstall.
set "QT_PREFIX=D:\Program\dev\Qt\6.8.3\msvc2022_64"

REM SDK_BIN is the Windows SDK binary directory that provides rc.exe and mt.exe.
REM Forward slashes are required because CMake treats backslashes as escape characters.
set "SDK_BIN=C:/Program Files (x86)/Windows Kits/10/bin/10.0.26100.0/x64"

REM SDK_ROOT is the Windows SDK root that contains include and library folders.
set "SDK_ROOT=C:\Program Files (x86)\Windows Kits\10"

REM SDK_VERSION is the installed Windows SDK version used for ABCNote builds.
set "SDK_VERSION=10.0.26100.0"

REM BUILD_DIR is the Release build directory used for deployable packages.
set "BUILD_DIR=%PROJECT_DIR%\build-msvc-qt-release"

REM Load MSVC compiler, linker, and standard library environment variables.
call "%VS_VCVARS%"
if errorlevel 1 exit /b %errorlevel%

REM PATH receives SDK tools because this vcvars installation does not add them automatically.
set "PATH=%SDK_ROOT%\bin\%SDK_VERSION%\x64;%PATH%"

REM INCLUDE receives SDK headers required by Windows and Universal CRT APIs.
set "INCLUDE=%SDK_ROOT%\Include\%SDK_VERSION%\ucrt;%SDK_ROOT%\Include\%SDK_VERSION%\um;%SDK_ROOT%\Include\%SDK_VERSION%\shared;%SDK_ROOT%\Include\%SDK_VERSION%\winrt;%SDK_ROOT%\Include\%SDK_VERSION%\cppwinrt;%INCLUDE%"

REM LIB receives SDK libraries such as kernel32.lib and ucrt.lib for linking.
set "LIB=%SDK_ROOT%\Lib\%SDK_VERSION%\um\x64;%SDK_ROOT%\Lib\%SDK_VERSION%\ucrt\x64;%LIB%"

REM Configure the project in Release mode.
"%CMAKE_EXE%" -S "%PROJECT_DIR%" -B "%BUILD_DIR%" -G Ninja -DCMAKE_PREFIX_PATH="%QT_PREFIX%" -DCMAKE_BUILD_TYPE=Release -DCMAKE_RC_COMPILER="%SDK_BIN%/rc.exe" -DCMAKE_MT="%SDK_BIN%/mt.exe"
exit /b %errorlevel%
