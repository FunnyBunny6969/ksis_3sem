#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

int main() {
    // Создание сокета
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        cout << "Socket creation failed" << endl;
        return 1;
    }

    // Настройка адреса
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1280);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Привязка сокета
    if (::bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        cout << "Bind failed" << endl;
        close(server_socket);
        return 1;
    }

    // Прослушивание порта
    if (listen(server_socket, 5) == -1) {
        cout << "Listen failed" << endl;
        close(server_socket);
        return 1;
    }

    cout << "Server started on port 1280..." << endl;

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        // Принятие соединения
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            cout << "Accept failed" << endl;
            continue;
        }

        cout << "Client connected!" << endl;

        char buf[255];
        int bytesReceived;
        
        while ((bytesReceived = recv(client_socket, buf, sizeof(buf) - 1, 0)) > 0) {
            buf[bytesReceived] = '\0';
            cout << "Received: " << buf << endl;
            
            // Подсчет букв 'a'
            int count_a = 0;
            int len = strlen(buf);
            for (int j = 0; j < len; j++) {
                if (buf[j] == 'a') count_a++;
            }
            
            // Переворачиваем строку
            char reversed[255];
            for (int i = 0; i < len; i++) {
                reversed[i] = buf[len - 1 - i];
            }
            reversed[len] = '\0';
            
            // Формируем ответ
            char response[512];
            sprintf(response, "%d %s", count_a, reversed);
            
            cout << "Sending response: " << response << endl;
            
            // Отправляем ответ
            send(client_socket, response, strlen(response), 0);
        }

        cout << "Client disconnected." << endl;
        close(client_socket);
    }

    close(server_socket);
    return 0;
}
