#include <iostream>
#include <winsock2.h>
#include <vector>
#include <string>
#include <cstdlib> // Äëÿ rand
#include <ctime>   // Äëÿ srand(time(0))

using namespace std;

void send_command(SOCKET sock, const string& command)
{
    send(sock, command.c_str(), command.size(), 0);
}

string recv_response(SOCKET sock)
{
    char buffer[1024];
    int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    buffer[bytes] = '\0';
    return string(buffer);
}

void client_process()
{
    WSADATA wsaData;
    SOCKET client_socket;
    sockaddr_in server_addr;

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET)
    {
        cout << "Socket creation failed!" << endl;
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4040);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        cout << "Connection failed!" << endl;
        return;
    }

    int n;
    cout << "Enter matrix size n: ";
    cin >> n;

    srand(time(0));
    vector<vector<int>> matrix(n, vector<int>(n));
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            matrix[i][j] = rand() % 100;
        }
    }

    cout << "Generated matrix:" << endl;
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            cout << matrix[i][j] << " ";
        }
        cout << endl;
    }


    string config = "CONFIG " + to_string(n) + " 2";
    send_command(client_socket, config);
    cout << recv_response(client_socket) << endl;

    send_command(client_socket, "MATRIX");
    for (int i = 0; i < n; ++i)
    {
        send(client_socket, (char*)matrix[i].data(), n * sizeof(int), 0);
    }
    cout << recv_response(client_socket) << endl;

    send_command(client_socket, "START");
    cout << recv_response(client_socket) << endl;

    send_command(client_socket, "STATUS");
    cout << "Server status: " << recv_response(client_socket) << endl;

    send_command(client_socket, "RESULT");
    vector<vector<int>> transposed(n, vector<int>(n));
    for (int i = 0; i < n; ++i)
    {
        recv(client_socket, (char*)transposed[i].data(), n * sizeof(int), 0);
    }

    cout << "Transposed matrix:" << endl;
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            cout << transposed[i][j] << " ";
        }
        cout << endl;
    }

    closesocket(client_socket);
    WSACleanup();
}

int main() {
    client_process();
    return 0;
}
