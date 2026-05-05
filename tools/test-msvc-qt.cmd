@echo off
REM This script runs ABCNote tests from the MSVC build directory.

REM SCRIPT_DIR is the absolute directory that contains this script.
set "SCRIPT_DIR=%~dp0"

REM PROJECT_DIR is the repository root, one level above the tools directory.
set "PROJECT_DIR=%SCRIPT_DIR%.."

REM CTEST_EXE is the CTest executable installed with CMake under D:\Program\dev.
set "CTEST_EXE=D:\Program\dev\CMake\bin\ctest.exe"

REM QT_BIN is the Qt runtime DLL directory required by ABCNoteTests.exe.
set "QT_BIN=D:\Program\dev\Qt\6.8.3\msvc2022_64\bin"

REM BUILD_DIR is the configured MSVC/Qt build directory.
set "BUILD_DIR=%PROJECT_DIR%\build-msvc-qt3"

REM PATH receives Qt DLLs so Windows can launch the test executable.
set "PATH=%QT_BIN%;%PATH%"

REM Run tests and print failing test output directly to the console.
"%CTEST_EXE%" --test-dir "%BUILD_DIR%" --output-on-failure
exit /b %errorlevel%
