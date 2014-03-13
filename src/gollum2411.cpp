#include <stdexcept>
#include <new>
#include <memory>

#include <sys/types.h>
#include <sys/socket.h>

#include <gollum2411.h>
#include <utils.h>

using std::string;
using std::shared_ptr;
using std::make_shared;

namespace gollum2411{
    Socket::Socket(int _sockfd, struct sockaddr_in _addr, int _domain,
                   int _type, int _protocol) :
    sockfd(_sockfd), addr(_addr), domain(_domain), type(_type),
    protocol(_protocol)
    {
        debug("Socket ctor(int, sockaddr, int, int, int)\n");
        int ret_val = -1;
        socklen_t socklen = 0;
        socklen = sizeof(addr);
        ret_val = getsockname(sockfd, (struct sockaddr*)&addr, &socklen);
        if(ret_val){
            throw socket_error("getsockname failed");
        }

        is_client = true;
        is_server = false;
        buf = NULL;
        buf = new char[size];

        ipaddr = inet_ntoa(addr.sin_addr);
        port = ntohs(addr.sin_port);
    }

    Socket::Socket(int _domain, int _type, int _protocol=0) :
    domain(_domain), type(_type), protocol(_protocol)
    {
        debug("Socket ctor (int, int, int)\n");
        sockfd = 0;
        sockfd = ::socket(domain, type, protocol);
        bzero(&addr, sizeof(addr));
        if(sockfd == -1){
            throw socket_error("socket");
        }
        buf = NULL;
        buf = new char[size];

        is_server = false;
        is_client = false;
    }

    Socket::~Socket(){
        debug("Socket dtor\n");
        if(sockfd > 0){
            debug("Closing %d\n", sockfd);
            ::close(sockfd);
        }

        if(buf){
            debug("Deleting buf pointer\n");
            delete []buf;
            buf = NULL;
        }
    }

    void Socket::bind(const int _port){
        int ret_val = -1;
        int sockopt = 0;
        socklen_t socklen = 0;

        if(is_client){
            throw socket_error("is_client = true");
        }

        is_server = true;
        port = _port;

        addr.sin_family = domain;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(_port);

        ret_val = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                            (const char *) &sockopt, sizeof(sockopt));
        if(ret_val){
            throw socket_error("setsockopt failed");
        }

        ret_val = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT,
                            (const char *) &sockopt, sizeof(sockopt));
        if(ret_val){
            throw socket_error("setsockopt failed");
        }

        ret_val = ::bind(sockfd, (const struct sockaddr*)&addr,
                   (socklen_t)sizeof(struct sockaddr_in));
        if(ret_val == -1){
            throw socket_error("bind failed");
        }

        socklen = sizeof(addr);
        ret_val = getsockname(sockfd, (struct sockaddr*)&addr, &socklen);
        if(ret_val){
            throw socket_error("getsockname failed");
        }

        ipaddr = inet_ntoa(addr.sin_addr);
        port = ntohs(addr.sin_port);

        debug("Bound to %s:%d\n", ipaddr.c_str(), port);
    }

    void Socket::connect(const char *ip, const int port){
        struct hostent *serverent = NULL;
        struct sockaddr_in to_addr;
        int ret_val = -1;

        if(is_server){
            throw socket_error("is_server = true");
        }
        is_client = true;

        bzero(&to_addr, sizeof(to_addr));

        serverent = gethostbyname(ip);
        if(!serverent){
            throw socket_error("gethostbyname failed");
        }

        bcopy((char *)serverent->h_addr,
           (char *)&to_addr.sin_addr.s_addr,
                serverent->h_length);
        to_addr.sin_family = domain;
        to_addr.sin_port = htons(port);
        ret_val = ::connect(sockfd, (struct sockaddr*)&(to_addr),
                            sizeof(struct sockaddr_in));
        if(ret_val){
            throw socket_error("connect failed");
        }
        debug("connect success\n");
    }

    void Socket::listen(const int backlog){
        int ret_val = ::listen(sockfd, backlog);
        if(ret_val){
            throw socket_error("listen failed");
        }
        debug("Listening...\n");
    }

    shared_ptr<Socket> Socket::accept(){
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        bzero(&client_addr, sizeof(client_addr));
        int client_fd = ::accept(sockfd,
                               (struct sockaddr*)&(client_addr),
                               &addr_len);
        if(client_fd == -1){
            throw socket_error("accept failed");
        }

        debug("Connection accepted\n");
        return shared_ptr<Socket>(new Socket(client_fd, client_addr, domain, type, protocol));
    }

    string Socket::recv(){
        bzero(buf, size);
        int bytes_read = ::read(sockfd, buf, size);
        if(bytes_read == -1){
            throw socket_error("read failed");
        }
        debug("recv success\n");
        return string(buf);
    }

    void Socket::send(string msg){
        int bytes_written = ::write(sockfd, msg.c_str(), msg.length());
        if(bytes_written == -1){
            throw socket_error("write failed");
        }
    }

    void Socket::close(){
        debug("Closing connections\n");
        if(sockfd){
            ::close(sockfd);
            sockfd = 0;
        }
    }

    int Socket::get_sockfd(){
        return sockfd;
    }

    socket_error::socket_error(const std::string& msg) :
        std::runtime_error(msg)
    {

    };
}
