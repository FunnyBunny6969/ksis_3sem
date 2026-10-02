@echo off
REM comment

echo Starting...
start "Servak" cmd /k server.exe
timeout /t 3 /nobreak >nul
rem start "Client 1" cmd /k client.exe
start "Client 1" cmd /k client.exe

REM taskkill /F /IM server.exe
