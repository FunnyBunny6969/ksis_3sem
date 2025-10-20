#include <stdio.h> 
#include <iostream> 
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Функция для безопасного получения данных от сервера
string receiveData(SOCKET s) {
    string full_response;
    char chunk[1024];
    bool message_complete = false;

    while (!message_complete) {
        int bytes_received = recv(s, chunk, sizeof(chunk) - 1, 0);

        if (bytes_received > 0) {
            chunk[bytes_received] = '\0';
            full_response.append(chunk);

            if (full_response.find("\nEND\n") != string::npos) {
                message_complete = true;
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

    return full_response;
}

// Функция для безопасной отправки команды
bool sendCommand(SOCKET s, const string& command) {
    int send_result = send(s, command.c_str(), command.length() + 1, 0);
    if (send_result == SOCKET_ERROR) {
        cout << "Send failed: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

void showMenu() {
    cout << "\n=== СИСТЕМА УПРАВЛЕНИЯ СТУДЕНТАМИ ===" << endl;
    cout << "1. Показать всех студентов" << endl;
    cout << "2. Найти по среднему баллу" << endl;
    cout << "3. Найти по ID" << endl;
    cout << "4. Добавить студента" << endl;
    cout << "5. Редактировать студента" << endl;
    cout << "6. Удалить студента" << endl;
    cout << "7. Справка" << endl;
    cout << "0. Выход" << endl;
    cout << "Выберите действие: ";
}

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

    cout << "Connected to server. Students management system ready." << endl;

    while (true) {
        showMenu();
        int choice;
        cin >> choice;
        cin.ignore(); // очищаем буфер

        string command;

        switch (choice) {
        case 1: // Показать всех
            command = "SHOW_ALL";
            break;

        case 2: { // Найти по среднему баллу
            double minAvg;
            cout << "Введите минимальный средний балл: ";
            cin >> minAvg;
            cin.ignore();
            command = "FIND_AVG " + to_string(minAvg);
            break;
        }

        case 3: { // Найти по ID
            int id;
            cout << "Введите ID студента: ";
            cin >> id;
            cin.ignore();
            command = "FIND_BY_ID " + to_string(id);
            break;
        }

        case 4: { // Добавить студента
            string name, group, grades;
            double scholarship;

            cout << "Введите ФИО: ";
            getline(cin, name);
            cout << "Введите группу: ";
            getline(cin, group);
            cout << "Введите стипендию: ";
            cin >> scholarship;
            cin.ignore();
            cout << "Введите оценки через запятую (например: 8,9,7,9): ";
            getline(cin, grades);

            command = "ADD " + name + "|" + group + "|" + to_string(scholarship) + "|" + grades;
            break;
        }

        case 5: { // Редактировать студента
            int id;
            string name, group, grades;
            double scholarship;

            cout << "Введите ID студента для редактирования: ";
            cin >> id;
            cin.ignore();
            cout << "Введите новое ФИО: ";
            getline(cin, name);
            cout << "Введите новую группу: ";
            getline(cin, group);
            cout << "Введите новую стипендию: ";
            cin >> scholarship;
            cin.ignore();
            cout << "Введите новые оценки через запятую: ";
            getline(cin, grades);

            command = "EDIT " + to_string(id) + " " + name + "|" + group + "|" + to_string(scholarship) + "|" + grades;
            break;
        }

        case 6: { // Удалить студента
            int id;
            cout << "Введите ID студента для удаления: ";
            cin >> id;
            cin.ignore();
            command = "DELETE " + to_string(id);
            break;
        }

        case 7: // Справка
            command = "HELP";
            break;

        case 0: // Выход
            cout << "Disconnecting..." << endl;
            closesocket(s);
            WSACleanup();
            return 0;

        default:
            cout << "Неверный выбор!" << endl;
            continue;
        }

        // Отправляем команду и получаем результат
        if (sendCommand(s, command)) {
            string response = receiveData(s);
            cout << "\n" << response << endl;
        }
        else {
            break; // Ошибка отправки
        }

        cout << "\nНажмите Enter для продолжения...";
        cin.get();
    }

    closesocket(s);
    WSACleanup();
    return 0;
}