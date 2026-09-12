#include <stdio.h> 
#include <iostream> 
#include <winsock2.h>
#include <vector>
#include <string>
#include <windows.h>
#include <ws2tcpip.h>
#include <sstream>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Структура студента
struct Student {
    int id;
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
int nextStudentId = 1;
int activeConnections = 0;
CRITICAL_SECTION cs;

void initializeStudents() {
    students = {
        {nextStudentId++, "Иванов Иван Иванович", "ГР-101", 1500.0, {8, 9, 7, 9, 8}},
        {nextStudentId++, "Петров Петр Петрович", "ГР-102", 1700.0, {9, 9, 10, 8, 9}},
        {nextStudentId++, "Сидорова Анна Сергеевна", "ГР-101", 1600.0, {7, 6, 8, 7, 7}},
        {nextStudentId++, "Козлов Дмитрий Алексеевич", "ГР-103", 1400.0, {6, 7, 6, 5, 7}},
        {nextStudentId++, "Николаева Екатерина Владимировна", "ГР-102", 1800.0, {10, 9, 10, 10, 9}}
    };
}

// Функция для парсинга оценок из строки
vector<int> parseGrades(const string& gradesStr) {
    vector<int> grades;
    stringstream ss(gradesStr);
    string grade;
    while (getline(ss, grade, ',')) {
        grades.push_back(stoi(grade));
    }
    return grades;
}

// Функция для форматирования оценок в строку
string formatGrades(const vector<int>& grades) {
    string result;
    for (size_t i = 0; i < grades.size(); ++i) {
        result += to_string(grades[i]);
        if (i < grades.size() - 1) result += ",";
    }
    return result;
}

// ПОКАЗАТЬ ВСЕХ СТУДЕНТОВ
string showAllStudents() {
    string result = "=== ВСЕ СТУДЕНТЫ ===\n";
    for (const auto& student : students) {
        result += "ID: " + to_string(student.id) + "\n";
        result += "ФИО: " + student.name + "\n";
        result += "Группа: " + student.group + "\n";
        result += "Стипендия: " + to_string(student.scholarship) + "\n";
        result += "Оценки: " + formatGrades(student.grades) + "\n";
        result += "Средний балл: " + to_string(student.getAverageGrade()) + "\n";
        result += "------------------------\n";
    }
    return result;
}

// ПОИСК СТУДЕНТОВ ПО СРЕДНЕМУ БАЛЛУ
string findStudentsAboveAverage(double minAverage) {
    string result = "=== СТУДЕНТЫ СО СРЕДНИМ БАЛЛОМ > " + to_string(minAverage) + " ===\n";
    bool found = false;

    for (const auto& student : students) {
        double average = student.getAverageGrade();
        if (average > minAverage) {
            found = true;
            result += "ID: " + to_string(student.id) + "\n";
            result += "ФИО: " + student.name + "\n";
            result += "Группа: " + student.group + "\n";
            result += "Стипендия: " + to_string(student.scholarship) + "\n";
            result += "Оценки: " + formatGrades(student.grades) + "\n";
            result += "Средний балл: " + to_string(average) + "\n";
            result += "------------------------\n";
        }
    }

    if (!found) {
        result = "Студентов со средним баллом выше " + to_string(minAverage) + " не найдено\n";
    }
    return result;
}

// ДОБАВИТЬ СТУДЕНТА
string addStudent(const string& name, const string& group, double scholarship, const string& gradesStr) {
    EnterCriticalSection(&cs);

    Student newStudent;
    newStudent.id = nextStudentId++;
    newStudent.name = name;
    newStudent.group = group;
    newStudent.scholarship = scholarship;
    newStudent.grades = parseGrades(gradesStr);

    students.push_back(newStudent);

    LeaveCriticalSection(&cs);

    return "Студент успешно добавлен! ID: " + to_string(newStudent.id) + "\n";
}

// РЕДАКТИРОВАТЬ СТУДЕНТА
string editStudent(int id, const string& name, const string& group, double scholarship, const string& gradesStr) {
    EnterCriticalSection(&cs);

    for (auto& student : students) {
        if (student.id == id) {
            student.name = name;
            student.group = group;
            student.scholarship = scholarship;
            student.grades = parseGrades(gradesStr);

            LeaveCriticalSection(&cs);
            return "Студент с ID " + to_string(id) + " успешно обновлен!\n";
        }
    }

    LeaveCriticalSection(&cs);
    return "Студент с ID " + to_string(id) + " не найден!\n";
}

// УДАЛИТЬ СТУДЕНТА
string deleteStudent(int id) {
    EnterCriticalSection(&cs);

    auto it = remove_if(students.begin(), students.end(),
        [id](const Student& s) { return s.id == id; });

    if (it != students.end()) {
        students.erase(it, students.end());
        LeaveCriticalSection(&cs);
        return "Студент с ID " + to_string(id) + " успешно удален!\n";
    }

    LeaveCriticalSection(&cs);
    return "Студент с ID " + to_string(id) + " не найден!\n";
}

// ПОИСК СТУДЕНТА ПО ID
string findStudentById(int id) {
    for (const auto& student : students) {
        if (student.id == id) {
            string result = "=== НАЙДЕН СТУДЕНТ ===\n";
            result += "ID: " + to_string(student.id) + "\n";
            result += "ФИО: " + student.name + "\n";
            result += "Группа: " + student.group + "\n";
            result += "Стипендия: " + to_string(student.scholarship) + "\n";
            result += "Оценки: " + formatGrades(student.grades) + "\n";
            result += "Средний балл: " + to_string(student.getAverageGrade()) + "\n";
            return result;
        }
    }
    return "Студент с ID " + to_string(id) + " не найден!\n";
}

// ОБРАБОТКА КОМАНД КЛИЕНТА
string processCommand(const string& command) {
    stringstream ss(command);
    string cmd;
    ss >> cmd;

    if (cmd == "SHOW_ALL") {
        return showAllStudents();
    }
    else if (cmd == "FIND_AVG") {
        double minAvg;
        ss >> minAvg;
        return findStudentsAboveAverage(minAvg);
    }
    else if (cmd == "FIND_BY_ID") {
        int id;
        ss >> id;
        return findStudentById(id);
    }
    else if (cmd == "ADD") {
        string name, group, gradesStr;
        double scholarship;

        // Читаем имя (может содержать пробелы)
        getline(ss, name, '|');
        getline(ss, group, '|');
        ss >> scholarship;
        ss.ignore(); // игнорируем пробел
        getline(ss, gradesStr);

        return addStudent(name.substr(1), group, scholarship, gradesStr); // убираем первый пробел
    }
    else if (cmd == "EDIT") {
        int id;
        string name, group, gradesStr;
        double scholarship;

        ss >> id;
        getline(ss, name, '|');
        getline(ss, group, '|');
        ss >> scholarship;
        ss.ignore();
        getline(ss, gradesStr);

        return editStudent(id, name.substr(1), group, scholarship, gradesStr);
    }
    else if (cmd == "DELETE") {
        int id;
        ss >> id;
        return deleteStudent(id);
    }
    else if (cmd == "HELP") {
        return "Доступные команды:\n"
            "SHOW_ALL - показать всех студентов\n"
            "FIND_AVG [балл] - найти по среднему баллу\n"
            "FIND_BY_ID [id] - найти по ID\n"
            "ADD [ФИО]|[группа]|[стипендия]|[оценки через запятую] - добавить\n"
            "EDIT [id] [ФИО]|[группа]|[стипендия]|[оценки] - редактировать\n"
            "DELETE [id] - удалить\n"
            "HELP - справка\n"
            "QUIT - выход\n";
    }
    else {
        return "Неизвестная команда. Введите HELP для справки.\n";
    }
}

DWORD WINAPI ThreadFunc(LPVOID lpParam) {
    SOCKET client_socket = *((SOCKET*)lpParam);
    delete (SOCKET*)lpParam;

    EnterCriticalSection(&cs);
    activeConnections++;
    cout << "Client connected. Active connections: " << activeConnections << endl;
    LeaveCriticalSection(&cs);

    char buf[1024];
    int bytes_received;

    DWORD timeout = 120000;
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

    while ((bytes_received = recv(client_socket, buf, sizeof(buf) - 1, 0)) > 0) {
        buf[bytes_received] = '\0';

        cout << "Команда от клиента: " << buf << endl;

        string result = processCommand(buf);

        // Безопасная отправка данных
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
    cout << "Available commands: SHOW_ALL, FIND_AVG, FIND_BY_ID, ADD, EDIT, DELETE, HELP" << endl;

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