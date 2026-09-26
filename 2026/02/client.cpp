#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

int main(void)
{
    char buf[200], b[200];

    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        cout << "WSAStartup failed: " << err << endl;
        return 1;
    }

    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET) {
        cout << "socket: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in add;
    memset(&add, 0, sizeof(add));
    add.sin_family = AF_INET;
    add.sin_port = htons(1024);
    add.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Тайм-аут на приём ответа — 3 секунды
    int timeout = 3000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,
               (const char*)&timeout, sizeof(timeout));

    int t = sizeof(add);

    cout << "Введите строку: ";
    cin.getline(buf, sizeof(buf), '\n');

    sendto(s, buf, (int)strlen(buf), 0,
           (struct sockaddr*)&add, t);

    int rv = recvfrom(s, b, sizeof(b) - 1, 0,
                      (struct sockaddr*)&add, &t);

    if (rv == SOCKET_ERROR) {
        int e = WSAGetLastError();
        if (e == WSAETIMEDOUT)
            cout << "Ответ не получен. Сервер запущен?" << endl;
        else
            cout << "recvfrom failed: " << e << endl;
    } else {
        b[rv] = '\0';
        cout << "Ответ сервера: " << b << endl;
    }

    closesocket(s);
    WSACleanup();
    return 0;
}