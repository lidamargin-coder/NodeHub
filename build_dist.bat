@echo off
setlocal enabledelayedexpansion
set "ROOT_DIR=%~dp0"
if "%ROOT_DIR:~-1%"=="\" set "ROOT_DIR=%ROOT_DIR:~0,-1%"

set "PATH=C:\Qt\6.8.3\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;%PATH%"

subst N: "%ROOT_DIR%" >nul 2>&1
if errorlevel 1 (
    subst N: /D >nul 2>&1
    subst N: "%ROOT_DIR%"
    if errorlevel 1 (
        echo Failed to map temporary drive N:
        exit /b 1
    )
)

echo [1/4] Configuring project...
cmake -S N:/ -B N:/build -G Ninja -DCMAKE_PREFIX_PATH="C:\Qt\6.8.3\mingw_64" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto fail

echo [2/4] Building NodeHub...
cmake --build N:/build --parallel
if errorlevel 1 goto fail

echo [3/4] Preparing standalone distribution folder...
if not exist "%ROOT_DIR%\dist\NodeHub" mkdir "%ROOT_DIR%\dist\NodeHub"
copy /Y "%ROOT_DIR%\build\NodeHub.exe" "%ROOT_DIR%\dist\NodeHub\NodeHub.exe" >nul

echo [4/4] Deploying Qt and MinGW runtime libraries...
windeployqt --qmldir N:/qml --no-translations --release "%ROOT_DIR%\dist\NodeHub\NodeHub.exe"
if errorlevel 1 goto fail

copy /Y "C:\Qt\Tools\mingw1310_64\bin\libgcc_s_seh-1.dll" "%ROOT_DIR%\dist\NodeHub\" >nul
copy /Y "C:\Qt\Tools\mingw1310_64\bin\libstdc++-6.dll" "%ROOT_DIR%\dist\NodeHub\" >nul
copy /Y "C:\Qt\Tools\mingw1310_64\bin\libwinpthread-1.dll" "%ROOT_DIR%\dist\NodeHub\" >nul

subst N: /D >nul 2>&1
echo ==========================================================
echo NodeHub Standalone Release Build Succeeded!
echo Standalone folder: %ROOT_DIR%\dist\NodeHub
echo Executable:       %ROOT_DIR%\dist\NodeHub\NodeHub.exe
echo ==========================================================
exit /b 0

:fail
subst N: /D >nul 2>&1
echo ==========================================================
echo Build failed!
echo ==========================================================
exit /b 1
