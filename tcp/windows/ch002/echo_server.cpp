#undef UNICODE

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x501 // getaddrinfo

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <thread>
#include <string>
//#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#define MAX_THREADS 2

using namespace std;

struct thread_data
{
    SOCKET ClientSocket;
    thread_data(SOCKET _ClientSocket) {
         ClientSocket = _ClientSocket;
    }
};

DWORD dwThreadIdArray[MAX_THREADS];
HANDLE hThreadArray[MAX_THREADS];

DWORD WINAPI thread_func(LPVOID lpParameter)
{
    int iResult;    
    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    thread_data *td = (thread_data*)lpParameter;// Receive until the peer shuts down the connection

    do {
        iResult = recv(td->ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
            printf("%d : Bytes received: %d\n", (int)td->ClientSocket, iResult);

            iSendResult = send(td->ClientSocket, recvbuf, iResult, 0);
            if(iSendResult == SOCKET_ERROR)
            {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(td->ClientSocket);
                //WSACleanup();
                return 1;
            }
            printf("Bytes send: %d\n", iSendResult);
        }
        else if (iResult == 0)
        {
            printf("Connection close...\n");
        }
        else
        {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(td->ClientSocket);
            //WSACleanup();
            return 1;
        }
    } while (iResult > 0);

    // shutdown the connection 
    iResult = shutdown(td->ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(td->ClientSocket);
        //WSACleanup();
        return 1;
    }

    // clean up
    closesocket(td->ClientSocket);
   
    //delete td;

    return 0;
}

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    
    ZeroMemory(dwThreadIdArray, sizeof(dwThreadIdArray));

    ZeroMemory(hThreadArray, sizeof(hThreadArray));

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if(iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints)); // 메모리 영역 0으로 초기화
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result); 
    if(iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        return 1;
    }

    // Create a socket for connencting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol); 

    if (ListenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result); // 할당한 것 해제
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int) result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result); // 할당한 것 해제
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    printf("listen localhost %s\n", DEFAULT_PORT);
    

    for(int i = 0; i < MAX_THREADS; i++)
    {
        // Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        cout << "ClientSocket: " << ClientSocket << " connected" << endl;
        hThreadArray[i] = CreateThread(NULL, 0, thread_func, new thread_data(ClientSocket), 0, &dwThreadIdArray[i]);
    }

    WaitForMultipleObjects(MAX_THREADS, hThreadArray, TRUE, INFINITE);
    
    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}