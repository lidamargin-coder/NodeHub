@echo off
setlocal enabledelayedexpansion
set "ROOT_DIR=%~dp0"
if "%ROOT_DIR:~-1%"=="\" set "ROOT_DIR=%ROOT_DIR:~0,-1%"

echo ==========================================================
echo [1/2] Building and deploying standalone distribution...
echo ==========================================================
call "%ROOT_DIR%\build_dist.bat"
if errorlevel 1 (
    echo Error during distribution build!
    exit /b 1
)

echo.
echo ==========================================================
echo [2/2] Compiling Windows Setup Installer with Inno Setup...
echo ==========================================================

set "ISCC_PATH=C:\Users\Р‘РѕРіРґР°С€Р°\AppData\Local\Programs\Inno Setup 6\ISCC.exe"
if not exist "%ISCC_PATH%" (
    set "ISCC_PATH=C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
)
if not exist "%ISCC_PATH%" (
    set "ISCC_PATH=C:\Program Files\Inno Setup 6\ISCC.exe"
)

if not exist "%ISCC_PATH%" (
    echo Error: Inno Setup compiler (ISCC.exe) not found!
    exit /b 1
)

"%ISCC_PATH%" "%ROOT_DIR%\installer\NodeHub_Setup.iss"
if errorlevel 1 (
    echo Error compiling installer!
    exit /b 1
)

echo.
echo ==========================================================
echo Setup Installer successfully built!
echo Installer path: %ROOT_DIR%\dist\NodeHub_Setup_v1.1.0.exe
echo ==========================================================
exit /b 0
