@echo off
REM Build ToyFrameV for Windows
REM Usage: build_windows.bat [clean] [debug|release] [target]

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set PROJECT_DIR=%SCRIPT_DIR%..
set BUILD_DIR=%PROJECT_DIR%\build
set CONFIG=Release
set TARGET=
set CLEAN=0

REM Parse arguments
:parse_args
if "%~1"=="" goto :end_parse
if /i "%~1"=="clean" set CLEAN=1
if /i "%~1"=="debug" set CONFIG=Debug
if /i "%~1"=="release" set CONFIG=Release
if /i "%~1"=="--target" (
    set TARGET=%~2
    shift
)
shift
goto :parse_args
:end_parse

echo.
echo ============================================
echo   Building ToyFrameV for Windows
echo ============================================
echo   Project: %PROJECT_DIR%
echo   Build:   %BUILD_DIR%
echo   Config:  %CONFIG%
echo.

REM Clean if requested
if %CLEAN%==1 (
    echo [*] Cleaning build directory...
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
)

REM Create build directory
if not exist "%BUILD_DIR%" (
    echo [*] Creating build directory...
    mkdir "%BUILD_DIR%"
)

cd /d "%BUILD_DIR%"

REM Configure with CMake
echo [*] Configuring with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo ERROR: CMake configuration failed
    exit /b 1
)

REM Build
echo [*] Building...
if "%TARGET%"=="" (
    cmake --build . --config %CONFIG%
) else (
    cmake --build . --config %CONFIG% --target %TARGET%
)
if errorlevel 1 (
    echo ERROR: Build failed
    exit /b 1
)

cd /d "%PROJECT_DIR%"

echo.
echo ============================================
echo   Build Complete!
echo ============================================
echo.
echo To run a sample:
echo   .\build\bin\%CONFIG%\HelloTriangle.exe
echo.

endlocal
