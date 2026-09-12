#include <winsock2.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
using namespace std;
#pragma comment(lib, "ws2_32.lib")




int main(){
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2,2);
    WSAStartup(wVersionRequested, &wsaData);

    struct sockaddr_in peer;
    peer.sin_family = AF_INET;
    peer.sin_port = htons(1280);
    peer.sin_addr.s_addr = inet_addr("127.0.0.1");

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

    connect(s, (struct sockaddr*)&peer, sizeof(peer));

    int m, n;
    cout << "Enter m and n: ";
    cin >> m >> n;
    // Отправляем оба числа одной строкой через пробел
    char buf[255];
    sprintf(buf, "%d %d", m, n);
    send(s, buf, (int)strlen(buf), 0);

    char b[255]; 
    int N = recv(s, b, sizeof(b)-1, 0);
    if (N > 0){
        b[N] = '\0';
        cout << b << endl;
    }

    closesocket(s);
    WSACleanup();
    return 0;
}