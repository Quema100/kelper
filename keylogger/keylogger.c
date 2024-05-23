#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 3000
#define BUFSIZE 1024

// ���� �Լ� ���� ��� �� ����
void err_quit(const char* msg)
{
    LPVOID lpMsgBuf = NULL;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
    exit(1);
}

// ���� �Լ� ���� ���
void err_display(const char* msg)
{
    LPVOID lpMsgBuf = NULL;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// Node.js ������ POST ��û ������
void send_post_request(const char* server_ip, int port, const char* path, const char* json_data)
{
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server = { 0 };
    char request[BUFSIZE];
    char response[BUFSIZE];
    int recv_size;

    // Winsock �ʱ�ȭ
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        err_quit("WSAStartup()");

    // ���� ����
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        err_quit("socket()");

    // ���� �ּ� ����
    server.sin_addr.s_addr = inet_addr(server_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    // ������ ����
    if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0)
        err_quit("connect()");

    // POST ��û �޽��� �ۼ�
    size_t content_length = strlen(json_data);
    snprintf(request, BUFSIZE,
        "POST %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n" 
        "Connection: close\r\n\r\n"
        "%s",
        path, server_ip, content_length, json_data); 

    // �޽��� ����
    if (send(s, request, (int)strlen(request), 0) < 0)
        err_quit("send()");

    // ���� ���� ����
    recv_size = recv(s, response, BUFSIZE, 0);
 
    if (recv_size == SOCKET_ERROR)
    {
        err_display("recv()");
    }
    else if (recv_size == 0)
    {
        printf("No more data received from the server.\n");
    }
    else
    {
        // ���� ���
        if (recv_size >= BUFSIZE)
        {
            // ���ŵ� �������� ũ�Ⱑ ������ ũ�⸦ �ʰ��ϴ� ���
            response[BUFSIZE - 1] = '\0'; // ������ ���� null ���� �߰�
        }
        else
        {
            response[recv_size] = '\0'; // ���ŵ� �������� ũ�⿡ �°� null ���� �߰�
        }
        printf("Response received:\n%s\n", response);
    }
    // ���� �ݱ�
    closesocket(s);
    WSACleanup();
}

int main(int argc, char* argv[])
{
    // Node.js ������ ���� JSON ������
    const char* json_data = "{\"logs\":\"value1\"}";

    // Node.js ������ POST ��û ������
    send_post_request("127.0.0.1", 3000, "/get_logs", json_data);

    return 0;
}
