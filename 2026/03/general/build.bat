@echo off
REM comment

echo Building...
g++ client.cpp -o client.exe -lws2_32
g++ ps.cpp -o ps.exe -lws2_32
g++ old_server.cpp -o old_server.exe -lws2_32