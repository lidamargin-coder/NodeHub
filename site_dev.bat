@echo off
setlocal
cd /d "%~dp0site"
echo ==========================================================
echo Starting NodeHub Website Dev Server (Vite + React)...
echo Opening http://localhost:5173 in your browser...
echo ==========================================================
call npm.cmd run dev -- --host --open
if errorlevel 1 (
    echo.
    echo Server stopped with error.
    pause
)
