#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

#define MAX_NAME_LEN 50
#define NUM_SUBJECTS 4

struct Student {
    char fullName[MAX_NAME_LEN];
    int groupNumber;
    double scholarship;
    int grades[NUM_SUBJECTS];
};

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 1;

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) { WSACleanup(); return 1; }

    sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(1280);
    inet_pton(AF_INET, "127.0.0.1", &dest_addr.sin_addr);

    if (connect(s, (sockaddr*)&dest_addr, sizeof(dest_addr)) == SOCKET_ERROR) {
        cout << "connect: " << WSAGetLastError() << endl;
        closesocket(s); WSACleanup(); return 1;
    }

    char buf[100];
    while (true) {
        cout << "Press Enter to get students without grade 3 (or type 'q' to quit):" << endl;
        cin.getline(buf, sizeof(buf));
        if (!cin.good() || buf[0] == 'q') break;

        // Отправляем запрос на сервер
        int len = (int)strlen(buf);
        if (send(s, buf, (len == 0 ? 1 : len), 0) == SOCKET_ERROR) break;

        // Получаем количество подходящих студентов
        int matchCount = 0;
        int n = recv(s, (char*)&matchCount, sizeof(matchCount), 0);
        if (n <= 0) break;

        cout << "\n--- Received from server (Count: " << matchCount << ") ---" << endl;

        // Получаем каждого студента по очереди
        for (int i = 0; i < matchCount; i++) {
            Student st;
            n = recv(s, (char*)&st, sizeof(Student), 0);
            if (n <= 0) break;

            cout << i + 1 << ". Name: " << st.fullName 
                 << " | Group: " << st.groupNumber 
                 << " | Scholarship: " << st.scholarship 
                 << " | Grades: ";
            for (int j = 0; j < NUM_SUBJECTS; j++) {
                cout << st.grades[j] << " ";
            }
            cout << endl;
        }
        cout << "------------------------------------------------\n" << endl;
    }

    shutdown(s, SD_SEND);
    closesocket(s);
    WSACleanup();
    return 0;
}