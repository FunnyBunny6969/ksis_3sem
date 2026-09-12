@echo off
echo Building server...

g++ server.cpp -o server.exe -lws2_32

if %errorlevel% neq 0 (
    echo Build FAILED.
) else (
    echo Build OK: server.exe
)



echo Building client...

g++ client.cpp -o client.exe -lws2_32

if %errorlevel% neq 0 (
    echo Build FAILED.
) else (
    echo Build OK: client.exe
)
