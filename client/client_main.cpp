#include <asio.hpp>
#include <iostream>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;
using asio::ip::tcp;

#define MAX_LEN 200
#define NUM_COLORS 6

bool exit_flag = false;
// void send_msg(int client_socket);
// void receive_msg(int client_socket);

int main(int argc, char *argv[]) {
    try {
        // Validate number of arguments
        if (argc != 3) {
            std::cerr << "Usage: client <host> <port>" << endl;
            return 1;
        }

        // All IO happens inside a context
        asio::io_context io_context;

        // Converting hostname port to endpoint address(es)
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints =
            resolver.resolve(argv[1], argv[2]);

        // create a socket
        tcp::socket socket(io_context);

        // connect to server
        asio::connect(socket, endpoints);
        asio::error_code error;
        fd_set read_fds;
        int sockfd = socket.native_handle(); // get the underlying socket descriptor
        int nfds = std::max(sockfd, 0) + 1;
        char name[MAX_LEN];
        cout << "Enter your name : ";
        int name_len = read(0, name, MAX_LEN);
        write(socket, asio::buffer(name, name_len), error);

        // asio::error_code error;
        for (;;) {
            FD_ZERO(&read_fds);
            FD_SET(0, &read_fds);
            FD_SET(sockfd, &read_fds);

            int count = select(nfds, &read_fds, nullptr, nullptr, nullptr);

            if (count < 0) {
                perror("select");
                break;
            }

            cout << "\n\t  **** Welcome to the chatroom ****   " << endl;
            if (FD_ISSET(0, &read_fds)) {
                // Some data on stdin
                while (1) {
                    cout << "You : ";
                    char msg[MAX_LEN];
                    int msg_len = read(0, msg, MAX_LEN);
                    int len = write(socket, asio::buffer(msg, msg_len));
                    if (strcmp(msg, "/exit") == 0) {
                        exit_flag = true;
                        socket.close();
                        continue;
                    }
                }
            }

            if (FD_ISSET(sockfd, &read_fds)) {
                // read data on socket
                while (1) {
                    if (exit_flag)
                        continue;
                    char name[MAX_LEN], msg[MAX_LEN];
                    int bytes_received = socket.read_some(asio::buffer(name));
                    if (bytes_received <= 0)
                        continue;
                    socket.read_some(asio::buffer(msg));
                    if (strcmp(name, "#NULL") != 0)
                        cout << name << " : " << msg << endl;
                    else
                        cout << msg << endl;
                    cout << "You : ";
                    fflush(stdout);
                }
                if (error == asio::error::eof)
                    break;
                else if (error)
                    throw asio::system_error(error);
            }
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << endl;
    }

    return 0;
}

// void send_msg(int client_socket) {
//     while (1) {
//         cout << "You : ";
//         char str[MAX_LEN];
//         cin.getline(str, MAX_LEN);
//         int len = write(socket, asio::buffer(str, sizeof(str)));
//         // send(client_socket, str, sizeof(str), 0);
//         if (strcmp(str, "#exit") == 0) {
//             exit_flag = true;
//             // t_recv.detach();
//             close(client_socket);
//             return;
//         }
//     }
// }

// void receive_msg(int client_socket) {
//     while (1) {
//         if (exit_flag)
//             return;
//         char name[MAX_LEN], str[MAX_LEN];
//         int bytes_recieved = socket.read_some(asio::buffer(name));
//         int bytes_received = recv(client_socket, name, sizeof(name), 0);
//         if (bytes_received <= 0)
//             continue;
//         recv(client_socket, str, sizeof(str), 0);

//         // eraseText(6);
//         if (strcmp(name, "#NULL") != 0)
//             cout << name << " : " << str << endl;
//         else
//             cout << str << endl;
//         cout << "You : ";
//         fflush(stdout);
//     }
// }