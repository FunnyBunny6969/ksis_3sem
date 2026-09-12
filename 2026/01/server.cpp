#include <winsock2.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

int main()
{
    // ===== 1. Инициализация WinSock =====
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(2, 2);
    WSAStartup(wVersionRequested, &wsaData);

    // ===== 2. Создание сокета =====
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        cout << "socket: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // ===== 3. Установка SO_REUSEADDR (чтобы перезапуск не падал с ошибкой 10048) =====
    BOOL reuse = TRUE;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
               (const char*)&reuse, sizeof(reuse));

    // ===== 4. Привязка сокета к адресу (bind) =====
    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons(1280);
    local.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR) {
        cout << "bind: " << WSAGetLastError() << endl;
        closesocket(s);
        WSACleanup();
        return 1;
    }

    // ===== 5. Перевод сокета в режим прослушивания (listen) =====
    int r = listen(s, SOMAXCONN);
    if (r == SOCKET_ERROR) {
        cout << "listen: " << WSAGetLastError() << endl;
        closesocket(s);
        WSACleanup();
        return 1;
    }

    cout << "Server is listening on port 1280..." << endl;

    // ===== 6. Главный цикл последовательного сервера =====
    while (true) {
        char buf[255], res[100], b[255], *Res;
        int n;

        // структура определяет удаленный адрес, с которым соединяется сокет
        sockaddr_in remote_addr;
        int size = sizeof(remote_addr);

        // Принимаем запрос на соединение
        SOCKET s2 = accept(s, (struct sockaddr*)&remote_addr, &size);
        if (s2 == INVALID_SOCKET) {
            cout << "accept: " << WSAGetLastError() << endl;
            continue;
        }

        cout << "Client connected!" << endl;

        // Работа с одним клиентом в цикле (пока он шлёт данные)
        while ((n = recv(s2, b, sizeof(b) - 1, 0)) > 0) {
            b[n] = '\0';

            // === ЛОГИКА ЗАДАЧИ: m! + n! ===
            int m = 0, nn = 0;
            sscanf(b, "%d %d", &m, &nn);

            long long fact_m = 1;
            for (int k = 2; k <= m; k++) fact_m *= k;

            long long fact_n = 1;
            for (int k = 2; k <= nn; k++) fact_n *= k;

            long long sum = fact_m + fact_n;

            sprintf(res, "%lld", sum);

            Res = new char[strlen(res) + 1];
            strcpy(Res, res);
            send(s2, Res, (int)strlen(Res), 0);
            delete[] Res;
        }

        // Закрываем соединение с конкретным клиентом
        closesocket(s2);
        cout << "Client disconnected." << endl;
    }

    // ===== 7. Завершение работы =====
    closesocket(s);
    WSACleanup();
    return 0;
}