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

bool exit_flag = false;
thread t_send, t_recv;
int client_socket;

// void catch_ctrl_c(int signal);
int eraseText(int cnt);
void send_message(int client_socket);
void recv_message(int client_socket);

int main(int argc, char *argv[])
{
    // if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    // {
    //     perror("socket: ");
    //     exit(-1);
    // }

    // struct sockaddr_in client;
    // client.sin_family = AF_INET;
    // client.sin_port = htons(32000); // Port no. of server
    // client.sin_addr.s_addr = INADDR_ANY;
    // // client.sin_addr.s_addr=inet_addr("127.0.0.1"); // Provide IP address of server
    // bzero(&client.sin_zero, 0);

    // if ((connect(client_socket, (struct sockaddr *)&client, sizeof(struct sockaddr_in))) == -1)
    // {
    //     perror("connect: ");
    //     exit(-1);
    // }
    if (argc != 3)
    {
        std::cerr << "Usage: client <host> <port>" << endl;
    }
    asio::io_context io_context;
    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve(argv[1], argv[2]);
    tcp::socket socket(io_context);
    asio::connect(socket, endpoints);
    int client_socket = socket.native_handle();
    // signal(SIGINT, catch_ctrl_c);
    char name[MAX_LEN];
    cout << "Enter your name : ";
    cin.getline(name, MAX_LEN);
    send(client_socket, name, sizeof(name), 0);

    cout << "\n\t  ====== Welcome to the chat-room ======   " << endl;

    thread t1(send_message, client_socket);
    thread t2(recv_message, client_socket);

    t_send = move(t1);
    t_recv = move(t2);

    if (t_send.joinable())
    {
        t_send.join();
    }
    if (t_recv.joinable())
    {
        t_recv.join();
    }
    return 0;
}

// Erase text from terminal
int eraseText(int cnt)
{
    char back_space = 8;
    for (int i = 0; i < cnt; i++)
    {
        cout << back_space;
    }
}

// Send message to everyone
void send_message(int client_socket)
{
    while (1)
    {
        cout << "You : ";
        char str[MAX_LEN];
        cin.getline(str, MAX_LEN);
        send(client_socket, str, sizeof(str), 0);
        if (strcmp(str, "#exit") == 0)
        {
            exit_flag = true;
            t_recv.detach();
            close(client_socket);
            return;
        }
    }
}

// Receive message
void recv_message(int client_socket)
{
    while (1)
    {
        if (exit_flag)
            return;
        char name[MAX_LEN], str[MAX_LEN];
        int color_code;
        int bytes_received = recv(client_socket, name, sizeof(name), 0);
        if (bytes_received <= 0)
            continue;
        recv(client_socket, &color_code, sizeof(color_code), 0);
        recv(client_socket, str, sizeof(str), 0);
        eraseText(6);
        if (strcmp(name, "#NULL") != 0)
            cout << name << " : " << str << endl;
        else
            cout << str << endl;
        cout << "You : ";
        fflush(stdout);
    }
}