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

        // БЕЗОПАСНАЯ ОТПРАВКА
        int send_result = send(s, buf, strlen(buf) + 1, 0);
        if (send_result == SOCKET_ERROR) {
            cout << "Send failed: " << WSAGetLastError() << endl;
            break;
        }

        // БЕЗОПАСНОЕ ПОЛУЧЕНИЕ
        string full_response;
        char chunk[1024];
        bool message_complete = false;

        while (!message_complete) {
            int bytes_received = recv(s, chunk, sizeof(chunk) - 1, 0);

            if (bytes_received > 0) {
                chunk[bytes_received] = '\0';
                full_response.append(chunk);

                // Проверяем маркер конца сообщения
                if (full_response.find("\nEND\n") != string::npos) {
                    message_complete = true;
                    // Убираем маркер из вывода
                    size_t pos = full_response.find("\nEND\n");
                    full_response = full_response.substr(0, pos);
                }
            }
            else if (bytes_received == 0) {
                cout << "Server disconnected" << endl;
                break;
            }
            else {
                if (WSAGetLastError() == WSAETIMEDOUT) {
                    cout << "Receive timeout" << endl;
                }
                else {
                    cout << "Receive error: " << WSAGetLastError() << endl;
                }
                break;
            }
        }

        if (!full_response.empty()) {
            cout << "Students found:" << endl;
            cout << full_response << endl;
        }
    }

    closesocket(s);
    WSACleanup();
    return 0;
}