@echo off


echo Building...

REM g++ client.cpp -o client.exe -lws2_32
REM g++ "client (2).cpp" -o client.exe -lws2_32
REM g++ server.cpp -o server.exe -lws2_32

g++ client_var3.cpp -o client.exe -lws2_32
g++ server_var3.cpp -o server.exe -lws2_32

echo Starting...

start "UDP Server" cmd /k server.exe
timeout /t 2 /nobreak >nul

start "UDP Client" cmd /k client.exe

REM taskkill /F /IM server.exe
