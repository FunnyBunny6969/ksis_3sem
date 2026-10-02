@echo off
REM comment

echo Starting...
start "Servak" cmd /k old_server.exe
timeout /t 3 /nobreak >nul
rem start "Client 1" cmd /k client.exe
start "Client 1" cmd /k "echo 111 | client.exe"
start "Client 2" cmd /k "echo 222 | client.exe"
start "Client 3" cmd /k "echo 333 | client.exe"

REM taskkill /F /IM server.exe
