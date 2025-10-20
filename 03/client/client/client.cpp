#include <stdio.h> 
#include <iostream> 
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {
    setlocale(LC_CTYPE, "Russian");

    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        return 1;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(1280);
    inet_pton(AF_INET, "127.0.0.1", &dest_addr.sin_addr.s_addr);

    if (connect(s, (sockaddr*)&dest_addr, sizeof(dest_addr)) == SOCKET_ERROR) {
        cout << "Connection failed!" << endl;
        closesocket(s);
        WSACleanup();
        return 1;
    }

    cout << "Connected to server. Enter minimum average grade (or 'quit' to exit):" << endl;

    while (true) {
        char buf[256];
        cout << "> ";
        cin.getline(buf, sizeof(buf));

        if (strcmp(buf, "quit") == 0) {
            cout << "Disconnecting..." << endl;
            break;
        }

        // Отправляем запрос серверу
        send(s, buf, strlen(buf) + 1, 0);

        // Получаем ответ от сервера
        char response[1024];
        int bytes_received = recv(s, response, sizeof(response), 0);

        if (bytes_received > 0) {
            cout << "Students found:" << endl;
            cout << response << endl;
        }
        else if (bytes_received == 0) {
            cout << "Server disconnected" << endl;
            break;
        }
        else {
            cout << "Receive error" << endl;
            break;
        }
    }

    closesocket(s);
    WSACleanup();
    return 0;
}