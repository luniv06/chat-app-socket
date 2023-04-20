#include <iostream>
#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <iostream>
#include <asio.hpp>

const int MAX_LEN = 200;

using namespace std;
using asio::ip::tcp;

class user_info
{
public:
    int userid;
    string username;
    int sockfd;
    thread th;
};

vector<user_info> users;
int uid = 0;
mutex mtx_ostream, mtx_users;

void print_for_all(string str, bool endLine);
void send_msg(string message, int sender_id);
void client_handler(int sockfd, int user_id);

int main()
{
    int portNo;
    cout << "Enter port no: ";
    cin >> portNo;

    int socket_id = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(portNo);
    server.sin_addr.s_addr = INADDR_ANY;
    bzero(&server.sin_zero, 0);
    bind(socket_id, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
    int check = listen(socket_id, 8);
    if (check < 0)
    {
        cout << "error: listen()" << endl;
        exit(1);
    }

    struct sockaddr_in client;
    unsigned int len = sizeof(sockaddr_in);
    cout << "\n\t  ***** Chat-Room server log *****   " << endl;

    for (;;)
    {
        int client_socket = accept(socket_id, (struct sockaddr *)&client, &len);
        if (client_socket < 0)
        {
            cout << "error: accept()" << endl;
            exit(1);
        }
        uid++;
        thread th(client_handler, client_socket, uid);
        lock_guard<mutex> guard(mtx_users);
        users.push_back({uid, string("client"), client_socket, (move(th))});
    }

    for (vector<user_info>::size_type i = 0; i < users.size(); i++)
    {
        if (users[i].th.joinable())
            users[i].th.join();
    }

    close(socket_id);
    return 0;
}

void client_handler(int sockfd, int user_id)
{
    char name[MAX_LEN], msg[MAX_LEN];
    recv(sockfd, name, sizeof(name), 0);

    // setting name of the client
    for (vector<user_info>::size_type i = 0; i < users.size(); i++)
    {
        if (users[i].userid == user_id)
        {
            users[i].username = string(name);
        }
    }

    // displaying welcome message
    string welcome = string(name) + string(" has joined");
    send_msg("user", user_id);
    send_msg(welcome, user_id);
    print_for_all(welcome, true);

    for (;;)
    {
        int len = recv(sockfd, msg, sizeof(msg), 0);
        if (len <= 0)
        {
            return;
        }
        if (strcmp(msg, "/exit") == 0)
        {
            // displaying leaving message
            string leave = string(name) + string(" has left");
            send_msg("user", user_id);
            send_msg(leave, user_id);
            print_for_all(leave, true);
            for (vector<user_info>::size_type i = 0; i < users.size(); i++)
            {
                if (users[i].userid == user_id)
                {
                    lock_guard<mutex> guard(mtx_users);
                    users[i].th.detach();
                    users.erase(users.begin() + i);
                    close(users[i].sockfd);
                    break;
                }
            }
            return;
        }
        send_msg(string(name), user_id);
        send_msg(string(msg), user_id);
        string message = string(name) + " : " + string(msg);
        print_for_all(message, true);
    }
}

// print common message for all
void print_for_all(string str, bool endLine)
{
    lock_guard<mutex> guard(mtx_ostream);
    cout << str;
    if (endLine)
    {
        cout << endl;
    }
}

// send the message to all clients except the sender
void send_msg(string message, int user_id)
{
    char msg[MAX_LEN];
    strcpy(msg, message.c_str());
    for (vector<user_info>::size_type i = 0; i < users.size(); i++)
    {
        if (users[i].userid != user_id)
        {
            send(users[i].sockfd, msg, sizeof(msg), 0);
        }
    }
}
