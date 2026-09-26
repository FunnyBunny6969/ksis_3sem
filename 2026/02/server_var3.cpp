#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cstring>
#include <string>
#include <cctype>

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

    cout << "Сервер запущен на порту 1024..." << endl;

    char b[200];
    sockaddr_in from;
    int fromlen = sizeof(from);

    while (true) {
        int rv = recvfrom(s, b, sizeof(b) - 1, 0,
                          (struct sockaddr*)&from, &fromlen);

        if (rv == SOCKET_ERROR) {
            cout << "recvfrom failed: " << WSAGetLastError() << endl;
            continue;
        }

        b[rv] = '\0';
        cout << "Получено: " << b << endl;

        // === Подсчёт букв слова WINDOWS ===
        const char letters[] = { 'W', 'I', 'N', 'D', 'O', 'S' };
        const int N = 6;
        int counts[6] = { 0 };

        for (int i = 0; i < rv; i++) {
            char c = toupper((unsigned char)b[i]);
            for (int j = 0; j < N; j++) {
                if (c == letters[j]) {
                    counts[j]++;
                }
            }
        }

        // Формируем ответ
        string answer = "Counts: ";
        bool allPresent = true;
        for (int j = 0; j < N; j++) {
            answer += letters[j];
            answer += "=";
            answer += to_string(counts[j]);
            answer += " ";
            if (counts[j] == 0) allPresent = false;
        }
        if (!allPresent) {
            answer += "| Some letters are missing!";
        }

        cout << "Отправлено: " << answer << endl;

        sendto(s, answer.c_str(), (int)answer.size(), 0,
               (struct sockaddr*)&from, fromlen);
    }

    closesocket(s);
    WSACleanup();
    return 0;
}