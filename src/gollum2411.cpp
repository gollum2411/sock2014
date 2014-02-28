#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>

#include <gollum2411.h>
#include <utils.h>

using std::string;

namespace gollum2411{
    TCPSocket::TCPSocket(){
        this->server_fd = 0;
        this->is_server = false;
        this->is_client = false;
        this->buf = new char[this->size];
        bzero(&(this->server_addr), sizeof(struct sockaddr_in));
        bzero(&(this->client_addr), sizeof(struct sockaddr_in));
    }

    TCPSocket::~TCPSocket(){

        if(server_fd > 0){
            ::close(server_fd);
        }

        if(client_fd > 0){
            ::close(client_fd);
        }

        if(this->buf){
            delete[] buf;
            buf = NULL;
        }

    }

    void TCPSocket::bind(const int port){
        int ret_val = -1;
        int sockopt = 0;

        if(this->is_client){
            throw socket_error("is_client = true");
        }

        this->is_server = true;

        this->server_addr.sin_family = AF_INET;
        this->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        this->server_addr.sin_port = htons(port);
        this->server_fd = socket(PF_INET, SOCK_STREAM, 0);
        if(this->server_fd == -1){
            throw socket_error("socket failed");
        }

        ret_val = setsockopt(this->server_fd, SOL_SOCKET, SO_REUSEADDR,
                            (const char *) &sockopt, sizeof(sockopt));
        if(ret_val){
            throw socket_error("setsockopt failed");
        }

        ret_val = ::bind(this->server_fd, (const struct sockaddr*)&server_addr,
                   (socklen_t)sizeof(struct sockaddr_in));
    }

    void TCPSocket::listen(const int backlog){
        int ret_val = ::listen(this->server_fd, backlog);
        if(ret_val){
            throw socket_error("listen failed");
        }
    }

    void TCPSocket::accept(){
        socklen_t addr_len = 0;
        this->client_fd = ::accept(this->server_fd,
                               (struct sockaddr*)&(this->client_addr),
                               &addr_len);
        if(this->client_fd == -1){
            throw socket_error("accept failed");
        }
    }

    void TCPSocket::send(string msg){
        int bytes_written = ::write(client_fd, msg.c_str(), msg.length());
        if(bytes_written == -1){
            throw socket_error("write failed");
        }
    }

    string TCPSocket::recv(){
        bzero(this->buf, this->size);
        int bytes_read = ::read(client_fd, this->buf, this->size);
        if(bytes_read == -1){
            throw socket_error("read failed");
        }
        return string(this->buf);
    }

    void TCPSocket::connect(const char *ip, const int port){
        struct hostent *serverent = NULL;
        int ret_val = -1;

        if(this->is_server){
            throw socket_error("is_server = true");
        }
        this->is_client = true;

        this->client_fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if(this->client_fd == -1){
            throw("socket failed");
        }

        serverent = gethostbyname(ip);
        if(!serverent){
            throw socket_error("gethostbyname failed");
        }

        memcpy(&(this->client_addr.sin_addr.s_addr), serverent->h_addr, serverent->h_length);
        ret_val = ::connect(this->client_fd, (struct sockaddr*)&(this->client_addr),
                            sizeof(struct sockaddr_in));
        if(ret_val){
            throw socket_error("connect failed");
        }
    }

    void TCPSocket::close(){
        if(this->is_server){
            ::close(this->client_fd);
        }

        if(this->is_client){
            ::close(this->client_fd);
        }
    }

    socket_error::socket_error(const std::string& msg) :
        std::runtime_error(msg)
    {

    };
}
