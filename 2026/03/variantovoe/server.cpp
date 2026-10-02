#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")
using namespace std;


#define MAX_NAME_LEN 50
#define NUM_SUBJECTS 4 // Например, 4 предмета

struct Student {
    char fullName[MAX_NAME_LEN];
    int groupNumber;
    double scholarship;
    int grades[NUM_SUBJECTS];
};

Student db[5] = {
    {"Ivanov I.I.",    101, 1500.0, {4, 5, 4, 5}}, 
    {"Petrov P.P.",    101, 1200.0, {3, 4, 5, 4}}, 
    {"Sidorova A.A.",  102, 1800.0, {5, 5, 5, 5}}, 
    {"Smirnov V.V.",   102, 1000.0, {3, 3, 4, 4}}, 
    {"Kuznetsova E.A.",101, 1600.0, {4, 4, 4, 5}}  
};

DWORD WINAPI ThreadFunc(LPVOID param){
    //Забираем СВОЮ копию дескриптора и освобождаем память.
    SOCKET s2 = *(SOCKET*)param;
    delete (SOCKET*)param;
    char buf[100], buf1[100];
    int n;

    while ((n = recv(s2, buf, sizeof(buf) - 1, 0)) > 0) {
        buf[n] = '\0';
        cout << "Received request from client: " << buf << endl;

        // Фильтрация: ищем студентов без оценки 3
        Student matching[5];
        int matchCount = 0;

        for (int i = 0; i < 5; i++) {
            bool hasThree = false;
            for (int j = 0; j < NUM_SUBJECTS; j++) {
                if (db[i].grades[j] == 3) {
                    hasThree = true;
                    break;
                }
            }
            if (!hasThree) {
                matching[matchCount++] = db[i];
            }
        }

        send(s2, (char*)&matchCount, sizeof(matchCount), 0);

        for (int i = 0; i < matchCount; i++) {
            send(s2, (char*)&matching[i], sizeof(Student), 0);
        }
    }
    closesocket(s2);
    return 0;
}


int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 1;
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) { WSACleanup(); return 1; }
    
    sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family      = AF_INET;
    local_addr.sin_port        = htons(1280);
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(s, (sockaddr*)&local_addr, sizeof(local_addr))
          == SOCKET_ERROR) {
          cout << "bind failed: " << WSAGetLastError() << endl;
        closesocket(s); WSACleanup(); return 1;
    }
    if (listen(s, SOMAXCONN) == SOCKET_ERROR) {
          cout << "listen: " << WSAGetLastError() << endl;
        closesocket(s); WSACleanup(); return 1;
    }
    
    cout << "Server receive ready" << endl << endl;
    while (true) {
        sockaddr_in client_addr;
        //Размер обязательно восстанавливаем перед КАЖДЫМ accept:
        //это одновременно входной и выходной параметр.
        int client_addr_size = sizeof(client_addr);
        SOCKET client_socket = accept(s,
            (sockaddr*)&client_addr, &client_addr_size);
        //accept при ошибке возвращает INVALID_SOCKET (все биты
        //единицы) — значение ИСТИННОЕ. Проверять надо явно.
        if (client_socket == INVALID_SOCKET) {
            cout << "accept: " << WSAGetLastError() << endl;
            break;
        }
        //Каждому потоку — СВОЯ копия дескриптора 
        SOCKET* arg = new SOCKET(client_socket);
        HANDLE h = CreateThread(NULL, 0, ThreadFunc,
                                arg, 0, NULL);
        if (h) {
            CloseHandle(h);      //иначе описатели накапливаются
        } else {
            closesocket(client_socket);
            delete arg;
        }
    }
    closesocket(s);
    WSACleanup();
    return 0;
}