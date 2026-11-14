@echo off
setlocal
set "ROOT_DIR=%~dp0"
if "%ROOT_DIR:~-1%"=="\" set "ROOT_DIR=%ROOT_DIR:~0,-1%"

set "PATH=C:\Qt\6.8.3\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;%PATH%"

subst N: "%ROOT_DIR%"
if errorlevel 1 (
    echo Failed to map temporary drive N:
    exit /b 1
)

cmake -S N:/ -B N:/build -G Ninja -DCMAKE_PREFIX_PATH="C:\Qt\6.8.3\mingw_64" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto fail

cmake --build N:/build --parallel
if errorlevel 1 goto fail

windeployqt --qmldir N:/qml --no-translations N:/build/NodeHub.exe
if errorlevel 1 goto fail

subst N: /D
echo Build succeeded! Executable: %ROOT_DIR%\build\NodeHub.exe
exit /b 0

:fail
subst N: /D
echo Build failed!
exit /b 1
