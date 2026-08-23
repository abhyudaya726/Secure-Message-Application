#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
    // 1. Initialize Winsock
    WSADATA wsaData;

    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0)
    {
        cerr << "WSAStartup failed. Error: "
             << result << endl;

        return 1;
    }

    cout << "Winsock initialized successfully."
         << endl;


    // 2. Create a socket
    SOCKET clientSocket = socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );

    if (clientSocket == INVALID_SOCKET)
    {
        cerr << "Socket creation failed. Error: "
             << WSAGetLastError() << endl;

        WSACleanup();
        return 1;
    }

    cout << "Client socket created successfully."
         << endl;


    // 3. Define the server address
    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(54000);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &serverAddress.sin_addr
    );


    // 4. Connect to the server
    result = connect(
        clientSocket,
        reinterpret_cast<sockaddr*>(&serverAddress),
        sizeof(serverAddress)
    );

    if (result == SOCKET_ERROR)
    {
        cerr << "Connection failed. Error: "
             << WSAGetLastError() << endl;

        closesocket(clientSocket);
        WSACleanup();

        return 1;
    }

    cout << "Connected to server successfully."
         << endl;


    // 5. Send a message to the server
    string message;
    cout<<"Enter message: ";
    getline(cin, message);

    int bytesSent = send(
        clientSocket,
        message.c_str(),
        static_cast<int>(message.length()),
        0
    );

    if (bytesSent == SOCKET_ERROR)
    {
        cerr << "Send failed. Error: "
            << WSAGetLastError() << endl;

        closesocket(clientSocket);
        WSACleanup();

        return 1;
    }

    cout << "Message sent to server."
        << endl;


    // 6. Receive the server's response
    char buffer[1024];

    int bytesReceived = recv(
        clientSocket,
        buffer,
        sizeof(buffer) - 1,
        0
    );

    if (bytesReceived == SOCKET_ERROR)
    {
        cerr << "Receive failed. Error: "
            << WSAGetLastError() << endl;

        closesocket(clientSocket);
        WSACleanup();

        return 1;
    }

    buffer[bytesReceived] = '\0';

    cout << "Server: " << buffer << endl;


    // 7. Clean up
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}