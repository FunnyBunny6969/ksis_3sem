#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

int main() {
    // Создание UDP сокета
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
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

    cout << "UDP server started on port 1280..." << endl;

    char buf[255];
    char response[512];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (true) {
        // Получение данных от клиента
        int bytesReceived = recvfrom(server_socket, buf, sizeof(buf) - 1, 0,
                                     (struct sockaddr*)&client_addr, &client_len);
        if (bytesReceived <= 0) continue;

        buf[bytesReceived] = '\0';
        cout << "Received: " << buf << endl;

        int len = strlen(buf);
char result[255];

// Если длина чётная и больше 5 (иначе нечего удалять)
if (len % 2 == 0 && len > 5) {
    int new_len = len - 5; // удаляем 3+2 символа
    strncpy(result, buf + 3, new_len); // копируем без первых 3 символов
    result[new_len] = '\0'; // завершаем строку нулём
} else {
    strcpy(result, buf); // если нечётная или слишком короткая, оставляем как есть
}

// Отправка клиенту
sendto(server_socket, result, strlen(result), 0,
       (struct sockaddr*)&client_addr, client_len);
    }

    close(server_socket);
    return 0;
}

