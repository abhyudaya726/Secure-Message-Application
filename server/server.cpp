#include <iostream>
#include <winsock2.h>
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
    SOCKET serverSocket = socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );

    if (serverSocket == INVALID_SOCKET)
    {
        cerr << "Socket creation failed. Error: "
             << WSAGetLastError() << endl;

        WSACleanup();
        return 1;
    }

    cout << "Server socket created successfully."
         << endl;


    // 3. Define the server address
    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(54000);


    // 4. Bind the socket to the address and port
    result = bind(
        serverSocket,
        reinterpret_cast<sockaddr*>(&serverAddress),
        sizeof(serverAddress)
    );

    if (result == SOCKET_ERROR)
    {
        cerr << "Bind failed. Error: "
             << WSAGetLastError() << endl;

        closesocket(serverSocket);
        WSACleanup();

        return 1;
    }

    cout << "Socket bound to port 54000."
         << endl;


    // 5. Start listening for connections
    result = listen(serverSocket, SOMAXCONN);

    if (result == SOCKET_ERROR)
    {
        cerr << "Listen failed. Error: "
             << WSAGetLastError() << endl;

        closesocket(serverSocket);
        WSACleanup();

        return 1;
    }

    cout << "Server is listening..."
         << endl;


    // 6. Accept an incoming client connection
    sockaddr_in clientAddress{};
    int clientAddressSize = sizeof(clientAddress);

    SOCKET clientSocket = accept(
        serverSocket,
        reinterpret_cast<sockaddr*>(&clientAddress),
        &clientAddressSize
    );

    if (clientSocket == INVALID_SOCKET)
    {
        cerr << "Accept failed. Error: "
            << WSAGetLastError() << endl;

        closesocket(serverSocket);
        WSACleanup();

        return 1;
    }

    cout << "Client connected successfully."
        << endl;


    // 7. Receive a message from the client
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
        closesocket(serverSocket);
        WSACleanup();

        return 1;
    }

    buffer[bytesReceived] = '\0';

    cout << "Client: " << buffer << endl;


    // 8. Send a response to the client
    const char* response = "Hello Client!";

    int bytesSent = send(
        clientSocket,
        response,
        static_cast<int>(strlen(response)),
        0
    );

    if (bytesSent == SOCKET_ERROR)
    {
        cerr << "Send failed. Error: "
            << WSAGetLastError() << endl;

        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();

        return 1;
    }

    cout << "Response sent to client."
        << endl;


    // 9. Clean up
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}