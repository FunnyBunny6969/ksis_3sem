#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

int main(void)
{
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

    struct sockaddr_in ad;
    ad.sin_port = htons(1024);
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = htonl(INADDR_ANY);

    BOOL reuse = TRUE;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
               (const char*)&reuse, sizeof(reuse));

    if (bind(s, (struct sockaddr*)&ad, sizeof(ad)) == SOCKET_ERROR) {
        cout << "bind: " << WSAGetLastError() << endl;
        closesocket(s);
        WSACleanup();
        return 1;
    }

    char b[10001], tmp = '\0';
    sockaddr_in from;              // адрес клиента, отдельно от ad
    int fromlen = sizeof(from);

    cout << "Listening..." << endl;

    while (true) {                 // сервер обслуживает запросы в цикле
        int rv = recvfrom(s, b, sizeof(b) - 1, 0,
                          (struct sockaddr*)&from, &fromlen);

        if (rv == SOCKET_ERROR) {
            cout << "recvfrom failed: " << WSAGetLastError() << endl;
            continue;
        }

        b[rv] = '\0';
        cout << "Catch: " << b << endl;

        // Меняем местами символы на чётных и нечётных позициях
        for (int i = 0; i + 1 < rv; i += 2) {
            tmp = b[i];
            b[i] = b[i + 1];
            b[i + 1] = tmp;
        }

        cout << "Send: " << b << endl;

        sendto(s, b, rv, 0, (struct sockaddr*)&from, fromlen);
    }                              // конец цикла обслуживания

    closesocket(s);
    WSACleanup();
    return 0;
}