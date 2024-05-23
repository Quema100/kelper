#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 3000
#define BUFSIZE 1024

// 소켓 함수 오류 출력 후 종료
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

// 소켓 함수 오류 출력
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

// Node.js 서버에 POST 요청 보내기
void send_post_request(const char* server_ip, int port, const char* path, const char* json_data)
{
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server = { 0 };
    char request[BUFSIZE];
    char response[BUFSIZE];
    int recv_size;

    // Winsock 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        err_quit("WSAStartup()");

    // 소켓 생성
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        err_quit("socket()");

    // 서버 주소 설정
    server.sin_addr.s_addr = inet_addr(server_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    // 서버에 연결
    if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0)
        err_quit("connect()");

    // POST 요청 메시지 작성
    size_t content_length = strlen(json_data);
    snprintf(request, BUFSIZE,
        "POST %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n" 
        "Connection: close\r\n\r\n"
        "%s",
        path, server_ip, content_length, json_data); 

    // 메시지 전송
    if (send(s, request, (int)strlen(request), 0) < 0)
        err_quit("send()");

    // 서버 응답 수신
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
        // 응답 출력
        if (recv_size >= BUFSIZE)
        {
            // 수신된 데이터의 크기가 버퍼의 크기를 초과하는 경우
            response[BUFSIZE - 1] = '\0'; // 버퍼의 끝에 null 문자 추가
        }
        else
        {
            response[recv_size] = '\0'; // 수신된 데이터의 크기에 맞게 null 문자 추가
        }
        printf("Response received:\n%s\n", response);
    }
    // 소켓 닫기
    closesocket(s);
    WSACleanup();
}

int main(int argc, char* argv[])
{
    // Node.js 서버에 보낼 JSON 데이터
    const char* json_data = "{\"logs\":\"value1\"}";

    // Node.js 서버에 POST 요청 보내기
    send_post_request("127.0.0.1", 3000, "/get_logs", json_data);

    return 0;
}
