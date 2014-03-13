#ifndef GOLLUM_2411
#define GOLLUM_2411

#include <stdexcept>
#include <iostream>
#include <memory>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <utils.h>

using std::runtime_error;
using std::string;
using std::shared_ptr;

namespace gollum2411{
    class Socket{
        public:
            Socket(int _domain, int _type, int _protocol);
            Socket(int _sockfd, struct sockaddr_in _addr, int _domain,
                   int _type, int _protocol);
            ~Socket();
            void bind(const int port);
            void connect(const char *ip, const int port);
            void listen(const int backlog=5);
            shared_ptr<Socket> accept();
            string recv();
            void send(string msg);
            void close();
            int get_sockfd();
        private:
            static const size_t size = 30000;
            char *buf;
            int sockfd;
            struct sockaddr_in addr;
            string ipaddr;
            int port;
            bool is_client;
            bool is_server;
            int domain;
            int type;
            int protocol;
    };

    class socket_error : public runtime_error {
        public:
            socket_error(const std::string& msg); /*!<Class constructor.*/
    };
}

#endif
