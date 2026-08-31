#include <iostream>
#include <winsock2.h>
#include <cstring>
#include <string>
#include <thread>
#include <vector>
#include <mutex>


#pragma comment(lib, "ws2_32.lib")

using namespace std;

struct Client{
    SOCKET socket;
    string username;
};

vector<Client> clients;
mutex clientsMutex;

void broadcastMessage(const string& message);
void handleClient(SOCKET clientSocket);


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

    while(true){
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

            break;
        }

        cout << "Client connected successfully."<< endl;

        lock_guard<mutex> lock(clientsMutex);

        Client newClient;
        newClient.socket = clientSocket;
        newClient.username = "";
        clients.push_back(newClient);

        cout<<"Connected Clients: "<<clients.size()<<endl;

        // 7. Start the messaging loop
        thread clientThread(handleClient, clientSocket);
        clientThread.detach();
    }
    

    // 8. Clean up
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}

void handleClient(SOCKET clientSocket){
    char usernameBuffer[1024];

    int bytesReceived = recv(
        clientSocket,
        usernameBuffer,
        sizeof(usernameBuffer) - 1,
        0
    );
    if(bytesReceived <= 0){
        closesocket(clientSocket);
        return;
    }

    usernameBuffer[bytesReceived] = '\0';
    string username = usernameBuffer;
    cout<<"Client Username: "<<username<<endl;

    {
        lock_guard<mutex> lock(clientsMutex);

        for (Client& client : clients)
        {
            if (client.socket == clientSocket)
            {
                client.username = username;
                break;
            }
        }
    }

    while (true)
    {
        char buffer[1024];

        // Receive message from client
        int bytesReceived = recv(
            clientSocket,
            buffer,
            sizeof(buffer) - 1,
            0
        );

        if (bytesReceived == 0){
            break;
        }

        if (bytesReceived == SOCKET_ERROR)
        {
            int error = WSAGetLastError();
            if (error != WSAECONNRESET){
                cerr << "Receive failed. Error: "<< error << endl;
            }

            break;
        }

        buffer[bytesReceived] = '\0';

        cout << "Client: " << buffer << endl;


        //Broadcast message to all clients
        broadcastMessage(buffer);
    }

    //Remove Client
    lock_guard<mutex> lock(clientsMutex);
    for(auto it = clients.begin(); it != clients.end(); it++){
        if(it->socket == clientSocket){
            clients.erase(it);
            break;
        }
    }
    cout<<"Client Disconnected. Connected Clients: "<<clients.size()<<endl;

    closesocket(clientSocket);
}

void broadcastMessage(const string& message){
    lock_guard<mutex> lock(clientsMutex);

    for(Client client : clients){
        int bytesSent = send(
            client.socket,
            message.c_str(),
            static_cast<int>(message.length()),
            0
        );

        if(bytesSent == SOCKET_ERROR){
            cerr<<"Broadcast failed. Error: "<<WSAGetLastError()<<endl;
        }
    }
}