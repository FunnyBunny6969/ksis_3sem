#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

int main(){
    // Создание сокета
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        cout << "Socket creation failed" << endl;
        return 1;
    }

    // Настройка адреса сервера
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1280);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // Подключение к серверу
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cout << "Connection failed" << endl;
        close(sock);
        return 1;
    }

    cout << "Connected to server!" << endl;
    
    char buf[255], response[512];
    
    while (true) {
        cout << "Enter the string (or 'quit' to exit): ";
        cin.getline(buf, 100);
        
        if (strcmp(buf, "quit") == 0) break;
        
        // Отправка данных
        if (send(sock, buf, strlen(buf) + 1, 0) < 0) {
            cout << "Send failed" << endl;
            break;
        } else {
            cout << "String sent: " << buf << endl;
            
            // Получение ответа
            int bytesReceived = recv(sock, response, sizeof(response) - 1, 0);
            if (bytesReceived > 0) {
                response[bytesReceived] = '\0';
                cout << "Server response:\n" << response << endl;
            } else {
                cout << "Receive failed or connection closed" << endl;
                break;
            }
        }
        cout << "------------------------" << endl;
    }

    close(sock);
    cout << "Disconnected from server." << endl;
    return 0;
}
