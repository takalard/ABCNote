@echo off
REM This script builds ABCNote Release after configure-msvc-qt-release.cmd has generated the build tree.

REM SCRIPT_DIR is the absolute directory that contains this script.
set "SCRIPT_DIR=%~dp0"

REM PROJECT_DIR is the repository root, one level above the tools directory.
set "PROJECT_DIR=%SCRIPT_DIR%.."

REM VS_VCVARS is the Visual Studio environment setup script for x64 compilation.
set "VS_VCVARS=D:\Program\dev\MicrosoftVisualStudio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

REM CMAKE_EXE is the CMake executable installed under D:\Program\dev.
set "CMAKE_EXE=D:\Program\dev\CMake\bin\cmake.exe"

REM BUILD_DIR is the Release build directory used for deployable packages.
set "BUILD_DIR=%PROJECT_DIR%\build-msvc-qt-release"

REM SDK_ROOT is the Windows SDK root that contains include and library folders.
set "SDK_ROOT=C:\Program Files (x86)\Windows Kits\10"

REM SDK_VERSION is the installed Windows SDK version used for ABCNote builds.
set "SDK_VERSION=10.0.26100.0"

REM Load MSVC compiler and linker environment variables.
call "%VS_VCVARS%"
if errorlevel 1 exit /b %errorlevel%

REM PATH receives SDK tools because this vcvars installation does not add them automatically.
set "PATH=%SDK_ROOT%\bin\%SDK_VERSION%\x64;%PATH%"

REM INCLUDE receives SDK headers required by Windows and Universal CRT APIs.
set "INCLUDE=%SDK_ROOT%\Include\%SDK_VERSION%\ucrt;%SDK_ROOT%\Include\%SDK_VERSION%\um;%SDK_ROOT%\Include\%SDK_VERSION%\shared;%SDK_ROOT%\Include\%SDK_VERSION%\winrt;%SDK_ROOT%\Include\%SDK_VERSION%\cppwinrt;%INCLUDE%"

REM LIB receives SDK libraries such as kernel32.lib and ucrt.lib for linking.
set "LIB=%SDK_ROOT%\Lib\%SDK_VERSION%\um\x64;%SDK_ROOT%\Lib\%SDK_VERSION%\ucrt\x64;%LIB%"

REM Configure automatically when the build directory is missing its CMake cache.
if not exist "%BUILD_DIR%\CMakeCache.txt" call "%SCRIPT_DIR%configure-msvc-qt-release.cmd"
if errorlevel 1 exit /b %errorlevel%

REM Build every target in the configured Release tree.
"%CMAKE_EXE%" --build "%BUILD_DIR%"
exit /b %errorlevel%
