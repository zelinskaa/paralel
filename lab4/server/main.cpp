#include <iostream>
#include <winsock2.h>
#include <thread>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

vector<vector<int>> matrix;
vector<vector<int>> transposed;
bool calculation_done = false;
int matrix_size = 0;
int num_threads = 1;

vector<vector<int>> transpose_matrix(const vector<vector<int>>& m)
{
    int n = m.size();
    vector<vector<int>> t(n, vector<int>(n));
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            t[j][i] = m[i][j];
        }
    }
    return t;
}

void handle_client(SOCKET client_socket)
{
    char buffer[1024];
    while (true)
    {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) break;

        buffer[bytes_received] = '\0';
        string command(buffer);
        istringstream iss(command);
        string cmd;
        iss >> cmd;

        if (cmd == "CONFIG")
        {
            iss >> matrix_size >> num_threads;
            matrix.resize(matrix_size, vector<int>(matrix_size));
            calculation_done = false;
            send(client_socket, "CONFIG_OK", 9, 0);
        }
        else if (cmd == "MATRIX")
        {
            for (int i = 0; i < matrix_size; ++i)
            {
                recv(client_socket, (char*)matrix[i].data(), matrix_size * sizeof(int), 0);
            }
            send(client_socket, "MATRIX_OK", 9, 0);
        }
        else if (cmd == "START")
        {
            transposed = transpose_matrix(matrix);
            calculation_done = true;
            send(client_socket, "STARTED", 7, 0);
        }
        else if (cmd == "STATUS")
        {
            if (calculation_done)
            {
                send(client_socket, "DONE", 4, 0);
            }
            else
            {
                send(client_socket, "PROCESSING", 10, 0);
            }

        }
        else if (cmd == "RESULT")
        {
            for (int i = 0; i < matrix_size; ++i)
            {
                send(client_socket, (char*)transposed[i].data(), matrix_size * sizeof(int), 0);
            }
        }
        else
        {
            send(client_socket, "UNKNOWN_COMMAND", 15, 0);
        }
    }
    closesocket(client_socket);
}

int main() {
    WSADATA wsaData;
    SOCKET server_socket, client_socket;
    sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET)
    {
        cout << "Socket creation failed!" << endl;
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(4040);

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        cout << "Bind failed!" << endl;
        return 1;
    }

    if (listen(server_socket, 5) == SOCKET_ERROR)
    {
        cout << "Listen failed!" << endl;
        return 1;
    }

    cout << "Server listening on port 4040..." << endl;

    while (true)
    {
        client_socket = accept(server_socket, (sockaddr*)&client_addr, &addr_len);
        if (client_socket == INVALID_SOCKET)
        {
            cout << "Accept failed!" << endl;
            continue;
        }
        thread(handle_client, client_socket).detach();
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
