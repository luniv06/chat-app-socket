#include <asio.hpp>
#include <ctime>
#include <errno.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>

using namespace std;
using asio::ip::tcp;

#define MAX_LEN 200

struct user
{
    int id;
    string name;
    tcp::socket *socket;
    // thread th;
};

vector<user> clients;
int uid = 0;
mutex cout_mtx, clients_mtx;

void set_name(int id, char name[]);
void print_for_all(string str, bool endLine);
int send_msg(string message, int sender_id);
int send_msg(int num, int sender_id);
void close_connection(int id);
void handle_client(tcp::socket *socket_ptr, int id);

int main()
{
    try
    {
        asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 5157));
        cout << "\n\t  **** Welcome to the chat-room ****   " << endl;
        for (;;)
        {
            tcp::socket *socket_ptr = new tcp::socket(io_context);
            acceptor.accept(*socket_ptr);
            uid++;
            std::thread t(handle_client, socket_ptr, uid);
            lock_guard<mutex> guard(clients_mtx);
            struct user temp = {
                uid,
                string("client"),
                socket_ptr,
            };
            clients.push_back(temp);
            t.detach();
            socket_ptr->close();
            delete socket_ptr;
        }
        // for (int i = 0; i < clients.size(); i++)
        // {
        //     if (clients[i].th.joinable())
        //     {
        //         clients[i].th.join();
        //     }
        // }
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

// Set name of client
void set_name(int id, char name[])
{
    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i].id == id)
        {
            clients[i].name = string(name);
        }
    }
}

void print_for_all(string str, bool flag = true)
{
    lock_guard<mutex> guard(cout_mtx);
    cout << str;
    if (flag)
        cout << endl;
}

// end connection for respective client
void close_connection(int id)
{
    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i].id == id)
        {
            //  clients[i].th.detach();
            lock_guard<mutex> guard(clients_mtx);
            clients.erase(clients.begin() + i);
            clients[i].socket->close();
            break;
        }
    }
}

// send message to all clients except the sender
int send_msg(tcp::socket *socket_ptr, string message, int client_id)
{
    char temp[MAX_LEN];
    strcpy(temp, message.c_str());
    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i].id != client_id)
        {
            socket_ptr->write_some(asio::buffer(temp, sizeof(temp)));
        }
    }
}

// send number to all clients except the sender
int send_msg(tcp::socket *socket_ptr, int num, int client_id)
{
    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i].id != client_id)
        {
            string temp = "" + num;
            char num_string[MAX_LEN];
            strcpy(num_string, temp.c_str());
            socket_ptr->write_some(asio::buffer(num_string, sizeof(num_string)));
        }
    }
}

// main function to handle a client
void handle_client(tcp::socket *socket_ptr, int id)
{
    char name[MAX_LEN], str[MAX_LEN];
    asio::error_code error;
    int len;
    len = socket_ptr->read_some(asio::buffer(name, MAX_LEN), error);
    set_name(id, name);

    string welcome_message = string(name) + string(" has joined");
    send_msg(socket_ptr, "#NULL", id);
    send_msg(socket_ptr, id, id);
    send_msg(socket_ptr, welcome_message, id);
    print_for_all(welcome_message);

    while (1)
    {
        int bytes_received = socket_ptr->read_some(asio::buffer(str, MAX_LEN), error);
        if (bytes_received <= 0)
            return;
        if (strcmp(str, "#exit") == 0)
        {
            // Display leaving message
            string message = string(name) + string(" has left");
            send_msg(socket_ptr, "#NULL", id);
            send_msg(socket_ptr, id, id);
            send_msg(socket_ptr, message, id);
            print_for_all(message);
            close_connection(id);
            return;
        }
        send_msg(socket_ptr, string(name), id);
        send_msg(socket_ptr, id, id);
        send_msg(socket_ptr, string(str), id);
        print_for_all(string(name) + " : " + string(str));
    }
}
