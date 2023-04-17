#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <signal.h>
#include <mutex>
#include <netinet/ip.h>
#include <asio.hpp>
#define MAX_LEN 200

using namespace std;
using asio::ip::tcp;

bool check_exit = false;
int socket_id;

int remove_text(int count);
void send_msg(int socket_id);
void receive_msg(int socket_id);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: client <host> <port>" << endl;
    }
    asio::io_context io_context;
    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve(argv[1], argv[2]);
    tcp::socket socket(io_context);
    asio::connect(socket, endpoints);

    int socket_id = socket.native_handle();
    fd_set read_fds;
    int nfds = std::max(socket_id, 0) + 1;

    char name[MAX_LEN];
    cout << "Enter username : ";
    cin.getline(name, MAX_LEN);
    send(socket_id, name, sizeof(name), 0);

    cout << "\n\t  **** Chat Room **** " << endl;

    for (;;)
    {
        FD_ZERO(&read_fds);
        FD_SET(0, &read_fds);
        FD_SET(socket_id, &read_fds);

        int count = select(nfds, &read_fds, nullptr, nullptr, nullptr);

        if (count < 0)
        {
            perror("select");
            break;
        }

        if (FD_ISSET(0, &read_fds))
        {
            // read data on stdin
            send_msg(socket_id);
        }

        if (FD_ISSET(socket_id, &read_fds))
        {
            // read data on socket.
            receive_msg(socket_id);
        }
    }
    return 0;
}

// remove text from terminal
int remove_text(int count)
{
    char back_space = 8;
    for (int i = 0; i < count; i++)
    {
        cout << back_space;
    }
}

// send message to all participants
void send_msg(int socket_id)
{
    cout << "You : ";
    char msg[MAX_LEN];
    cin.getline(msg, MAX_LEN);
    send(socket_id, msg, sizeof(msg), 0);
    if (strcmp(msg, "/exit") == 0)
    {
        check_exit = true;
        close(socket_id);
        return;
    }
}

// Receive message
void receive_msg(int socket_id)
{

    if (check_exit)
        return;
    char name[MAX_LEN], msg[MAX_LEN];
    int bytes_received = recv(socket_id, name, sizeof(name), 0);
    if (bytes_received <= 0)
        return;
    recv(socket_id, msg, sizeof(msg), 0);
    remove_text(6);
    if (strcmp(name, "user") != 0)
        cout << name << " : " << msg << endl;
    else
        cout << msg << endl;
    cout << "You : ";
    fflush(stdout);
}