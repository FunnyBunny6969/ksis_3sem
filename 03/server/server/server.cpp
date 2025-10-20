#include <stdio.h> 
#include <iostream> 
#include <winsock2.h>
#include <vector>
#include <string>
#include <windows.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Структура студента
struct Student {
    string name;
    string group;
    double scholarship;
    vector<int> grades;

    double getAverageGrade() const {
        if (grades.empty()) return 0;
        double sum = 0;
        for (int grade : grades) {
            sum += grade;
        }
        return sum / grades.size();
    }
};

vector<Student> students;
int activeConnections = 0;
CRITICAL_SECTION cs;

void initializeStudents() {
    students = {
        {"Иванов Иван Иванович", "ГР-101", 1500.0, {8, 9, 7, 9, 8}},
        {"Петров Петр Петрович", "ГР-102", 1700.0, {9, 9, 10, 8, 9}},
        {"Сидорова Анна Сергеевна", "ГР-101", 1600.0, {7, 6, 8, 7, 7}},
        {"Козлов Дмитрий Алексеевич", "ГР-103", 1400.0, {6, 7, 6, 5, 7}},
        {"Николаева Екатерина Владимировна", "ГР-102", 1800.0, {10, 9, 10, 10, 9}}
    };
}

string findStudentsAboveAverage(double minAverage) {
    string result;
    for (const auto& student : students) {
        double average = student.getAverageGrade();
        if (average > minAverage) {
            result += "ФИО: " + student.name + "\n";
            result += "Группа: " + student.group + "\n";
            result += "Стипендия: " + to_string(student.scholarship) + "\n";
            result += "Оценки: ";
            for (int grade : student.grades) {
                result += to_string(grade) + " ";
            }
            result += "\nСредний балл: " + to_string(average) + "\n";
            result += "------------------------\n";
        }
    }
    if (result.empty()) {
        result = "Студентов со средним баллом выше " + to_string(minAverage) + " не найдено\n";
    }
    return result;
}


DWORD WINAPI ThreadFunc(LPVOID lpParam) {
    SOCKET client_socket = *((SOCKET*)lpParam);
    delete (SOCKET*)lpParam;

    EnterCriticalSection(&cs);
    activeConnections++;
    cout << "Client connected. Active connections: " << activeConnections << endl;
    LeaveCriticalSection(&cs);

    char buf[256];
    int bytes_received;

    // Увеличиваем таймаут и добавляем таймаут на отправку
    DWORD timeout = 120000; // 2 минуты
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

    while ((bytes_received = recv(client_socket, buf, sizeof(buf) - 1, 0)) > 0) {
        buf[bytes_received] = '\0'; // Гарантируем строку

        double minAverage = atof(buf);
        cout << "Клиент запросил студентов со средним баллом > " << minAverage << endl;

        string result = findStudentsAboveAverage(minAverage);

        // БЕЗОПАСНАЯ ОТПРАВКА
        int total_sent = 0;
        const char* data = result.c_str();
        int data_len = result.length();

        while (total_sent < data_len) {
            int sent = send(client_socket, data + total_sent, data_len - total_sent, 0);
            if (sent == SOCKET_ERROR) {
                cout << "Send failed: " << WSAGetLastError() << endl;
                goto cleanup;
            }
            total_sent += sent;
        }

        // Отправляем маркер конца сообщения
        const char* end_marker = "\nEND\n";
        send(client_socket, end_marker, strlen(end_marker), 0);
    }

cleanup:
    shutdown(client_socket, SD_BOTH);
    closesocket(client_socket);

    EnterCriticalSection(&cs);
    activeConnections--;
    cout << "Client disconnected. Active connections: " << activeConnections << endl;
    LeaveCriticalSection(&cs);

    return 0;
}


int main() {
    setlocale(LC_CTYPE, "rus");
    initializeStudents();
    InitializeCriticalSection(&cs);

    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        cout << "WSAStartup failed" << endl;
        return 1;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

    int yes = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes));

    sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(1280);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    bind(s, (sockaddr*)&local_addr, sizeof(local_addr));
    listen(s, 5);

    cout << "Server started. Students database ready." << endl;
    cout << "Available students:" << endl;
    for (const auto& student : students) {
        cout << student.name << " - avg: " << student.getAverageGrade() << endl;
    }
    cout << endl;

    SOCKET client_socket;
    sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);

    cout << "Waiting for connections..." << endl;

    while (true) {
        client_socket = accept(s, (sockaddr*)&client_addr, &client_addr_size);

        if (client_socket == INVALID_SOCKET) {
            cout << "Accept error: " << WSAGetLastError() << endl;
            continue;
        }

        SOCKET* client_socket_ptr = new SOCKET(client_socket);
        HANDLE hThread = CreateThread(NULL, 0, ThreadFunc, client_socket_ptr, 0, NULL);

        if (hThread) {
            CloseHandle(hThread); 
        }
        else {
            cout << "Failed to create thread" << endl;
            closesocket(client_socket);
            delete client_socket_ptr;
        }
    }

    DeleteCriticalSection(&cs);
    closesocket(s);
    WSACleanup();
    return 0;
}