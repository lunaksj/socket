#define _WIN32_WINNT 0x501

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <new>


//#pragma comment (lib, "Ws2_32.lib")
//#pragma comment (lib, "Mswsock.lib")
//#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"


using namespace std;


struct thread_data
{
    SOCKET ConnectSocket;
    thread_data(SOCKET _ConnectSocket) {
         ConnectSocket = _ConnectSocket;
    }
};

DWORD WINAPI thread_func(LPVOID lpParameter)
{
    int iResult;    
    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    char sendbuf[DEFAULT_BUFLEN];
    //std::string sendData = "this is test";
    
   
    thread_data *td = (thread_data*)lpParameter;// Receive until the peer shuts down the connection

    strcpy(sendbuf, "test");
    
    //strcpy(sendbuf, (char *)td->ConnectSocket);
    // Send an initial buffer
    iResult = send( td->ConnectSocket, sendbuf, (int) strlen(sendbuf), 0);
    if(iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(td->ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Send: %d\n", iResult);


    // Recevie until the peer closes the connection
    int i = 0;
    do 
    {
        if (i > 1) break;
        iResult = recv(td->ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
            printf("Bytes received: %d\n", iResult);
            printf("%s\n", recvbuf);
            iSendResult = send( td->ConnectSocket, sendbuf, (int) strlen(sendbuf), 0);
            if(iSendResult == SOCKET_ERROR)
            {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(td->ConnectSocket);
                WSACleanup();
                return 1;
            }
        }
        else if (iResult == 0)
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());
        i++;
    } while (iResult > 0);


    // shutdown the connection since no more data will be sent
    iResult = shutdown(td->ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(td->ConnectSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(td->ConnectSocket);
    delete td;

    return 0;
}


int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    char sendbuf[DEFAULT_BUFLEN];
//    std::string sendData = "this is test";
    strcpy(sendbuf, "this is test");
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

    //cout << "Hello" << endl;
    //cout << sendData << endl;
    cout << sendbuf << endl;

    // Validate the parameters
    if (argc !=3)
    {
        printf("usage: %s server-name client_count\n", argv[0]);
        return 1;
    }

    // Initialize Winsoc
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    int client_count = atoi(argv[2]);
    //client_count = 1;
    DWORD *dwThreadIdArray = new DWORD[client_count];
    HANDLE *hThreadArray = new HANDLE[client_count];

    for(int i = 0; i < (int) client_count; i++)
    {
        // Attempt to connect to address until one succeeds
        for (ptr=result; ptr != NULL; ptr=ptr->ai_next)
        {
            // Create a SOCKET for connecting to server
            ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
            if (ConnectSocket == INVALID_SOCKET)
            {
                printf("socket failed with error: %d\n", WSAGetLastError());
                WSACleanup();
                return 1;
            }

            // Connect to server
            iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
            if(iResult == SOCKET_ERROR)
            {
                closesocket(ConnectSocket);
                ConnectSocket = INVALID_SOCKET;
                continue;
            }
            break;
        }


        if (ConnectSocket == INVALID_SOCKET)
        {
            printf("Unable to connect server!\n");
            freeaddrinfo(result);
            WSACleanup();
            return 1;
        }

         hThreadArray[i] = CreateThread(NULL, 0, thread_func, new thread_data(ConnectSocket), 0, &dwThreadIdArray[i]);

         
printf("%d, %d, %d\n", i, client_count, ConnectSocket);
    }

    WaitForMultipleObjects(client_count, hThreadArray, TRUE, INFINITE);
   
    freeaddrinfo(result);
    

    WSACleanup();

    delete[] hThreadArray;
    delete[] dwThreadIdArray;

    return 0;

}