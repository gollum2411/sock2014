#include <stdexcept>
#include <new>
#include <memory>
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>

#include <gollum2411.h>
#include <utils.h>

using std::string;
using std::stringstream;
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
        debug("sockfd = %d\n", sockfd);
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

    string Socket::recvfrom(struct sockaddr_in &response_addr){
        struct sockaddr_in rsock;
        socklen_t len = sizeof(rsock);
        bzero(&rsock, len);
        bzero(buf, size);
        int ret_val = ::recvfrom(sockfd, buf, size, 0, (struct sockaddr*)&rsock, &len);
        if(ret_val == -1){
            throw socket_error("recvfrom");
        }

        response_addr = rsock;
        return string(buf);
    }

    void Socket::sendto(string addr, const int port, string msg){
        struct sockaddr_in sockaddr;
        int ret_val = -1;
        bzero(&sockaddr, sizeof(sockaddr));

        sockaddr.sin_family = domain;
        sockaddr.sin_port = htons(port);
        ret_val = inet_pton(domain, addr.c_str(), &sockaddr.sin_addr.s_addr);
        if(ret_val!=1){
            throw socket_error("inet_aton");
        }

        socklen_t len = sizeof(sockaddr);
        int bytes = ::sendto(sockfd, msg.c_str(), msg.length(), 0,
                             (const struct sockaddr*)&sockaddr, len);
        if(bytes == -1){
            stringstream ss;
            ss << "sendto : " << errno  << " : " << strerror(errno);
            throw socket_error(ss.str());
        }
        debug("sendto succeeded\n");
        return;
    }

    void Socket::sendto(struct sockaddr_in addr, string msg){
        char buf[255] = {0};
        if(inet_ntop(AF_INET, &addr.sin_addr, buf, sizeof(buf)) == NULL){
            throw socket_error("inet_ntop failed");
        }

        int port = ntohs(addr.sin_port);

        sendto(buf, port, msg);
    }

    socket_error::socket_error(const std::string& msg) :
        std::runtime_error(msg)
    {

    };

    void write_to_ssl(SSL *ssl, string msg){
        if(SSL_write(ssl, (void*)msg.c_str(), msg.length()) != (int)msg.length()){
            throw socket_error("Failed writing to ssl");
        }
    }

    string read_from_ssl(SSL *ssl){
        char *buf = new char[BUFSIZE]();
        if(SSL_read(ssl, buf, BUFSIZE) <= 0){
            delete[] buf;
            throw socket_error("Failed reading from ssl");
        }
        string s = buf;
        delete[] buf;
        return s;
    }

}
