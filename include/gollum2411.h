#ifndef GOLLUM_2411
#define GOLLUM_2411

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdexcept>
#include <iostream>

#include <utils.h>

using std::runtime_error;
using std::string;

namespace gollum2411{
    class TCPSocket{
        public:
            TCPSocket();
            ~TCPSocket();
            void bind(const int port);
            void connect(const char *ip, const int port);
            void listen(const int backlog=5);
            void accept();
            string recv();
            void send(string msg);
            void close();
        private:
            static const size_t size = 30000;
            int server_fd;
            int client_fd;
            bool is_server;
            bool is_client;
            char *buf;
            struct sockaddr_in server_addr;
            struct sockaddr_in client_addr;
    };

    class socket_error : public runtime_error {
        public:
            socket_error(const std::string& msg);
    };
}

#endif
