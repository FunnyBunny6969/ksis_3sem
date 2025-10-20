#include <stdio.h> 
#include <iostream> 
#include <winsock2.h>
#include <vector>
#include <string>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Структура студента
struct Student {
    string name;
    string group;
    double scholarship;
    vector<int> grades;

    // Метод для вычисления среднего балла
    double getAverageGrade() const {
        if (grades.empty()) return 0;
        double sum = 0;
        for (int grade : grades) {
            sum += grade;
        }
        return sum / grades.size();
    }
};

// Глобальный список студентов
vector<Student> students;

// Инициализация тестовых данных
void initializeStudents() {
    students = {
        {"Иванов Иван Иванович", "ГР-101", 1500.0, {8, 9, 7, 9, 8}},
        {"Петров Петр Петрович", "ГР-102", 1700.0, {9, 9, 10, 8, 9}},
        {"Сидорова Анна Сергеевна", "ГР-101", 1600.0, {7, 6, 8, 7, 7}},
        {"Козлов Дмитрий Алексеевич", "ГР-103", 1400.0, {6, 7, 6, 5, 7}},
        {"Николаева Екатерина Владимировна", "ГР-102", 1800.0, {10, 9, 10, 10, 9}}
    };
}

// Функция для поиска студентов со средним баллом выше заданного
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

DWORD WINAPI ThreadFunc(LPVOID client_socket) {
    SOCKET s2 = *((SOCKET*)client_socket);
    char buf[256];

    while (recv(s2, buf, sizeof(buf), 0) > 0) {
        // Преобразуем полученные данные в число (минимальный средний балл)
        double minAverage = atof(buf);

        cout << "Клиент запросил студентов со средним баллом > " << minAverage << endl;

        // Ищем студентов
        string result = findStudentsAboveAverage(minAverage);

        // Отправляем результат клиенту
        send(s2, result.c_str(), result.length() + 1, 0);
    }

    closesocket(s2);
    return 0;
}

int numcl = 0;

void print() {
    if (numcl)
        printf("%d client connected\n", numcl);
    else
        printf("No clients connected\n");
}

int main() {
    // Инициализируем список студентов
    setlocale(LC_CTYPE, "rus");
    initializeStudents();

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
    sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(1280);
    local_addr.sin_addr.s_addr = 0;

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

    while ((client_socket = accept(s, (sockaddr*)&client_addr, &client_addr_size))) {
        numcl++;
        print();

        DWORD thID;
        CreateThread(NULL, NULL, ThreadFunc, &client_socket, NULL, &thID);
    }

    return 0;
}