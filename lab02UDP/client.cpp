#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

int main() {
    // Создание UDP сокета
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        cout << "Socket creation failed" << endl;
        return 1;
    }

    // Настройка адреса сервера
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1280);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    cout << "UDP client ready!" << endl;

    char buf[255], response[512];
    socklen_t addr_len = sizeof(server_addr);

    while (true) {
        cout << "Enter the string (or 'quit' to exit): ";
        cin.getline(buf, 100);

        if (strcmp(buf, "quit") == 0) break;

        // Отправка данных на сервер
        if (sendto(sock, buf, strlen(buf) + 1, 0,
                   (struct sockaddr*)&server_addr, addr_len) < 0) {
            cout << "Send failed" << endl;
            break;
        }

        // Получение ответа
        int bytesReceived = recvfrom(sock, response, sizeof(response) - 1, 0,
                                     (struct sockaddr*)&server_addr, &addr_len);
        if (bytesReceived > 0) {
            response[bytesReceived] = '\0';
            cout << "Server response:\n" << response << endl;
        } else {
            cout << "Receive failed" << endl;
            break;
        }

        cout << "------------------------" << endl;
    }

    close(sock);
    cout << "UDP client finished." << endl;
    return 0;
}
