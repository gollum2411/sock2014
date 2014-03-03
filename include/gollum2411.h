#ifndef GOLLUM_2411
#define GOLLUM_2411

#include <stdexcept>
#include <iostream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <utils.h>

using std::runtime_error;
using std::string;

namespace gollum2411{
    /**
     * Class to abstract socket behavior
    */
    class TCPSocket{
        public:
            TCPSocket(); /*!<Class constuctor. Initializes buffer and structs.*/
            ~TCPSocket(); /*!<Class destructor. Deletes buffer, closes socket file descriptors.*/
            void bind(const int port); /*!<Binds to port specified by port parameter.
                                        *@param[in] port Port to connect to.*/
            void connect(const char *ip, const int port); /*!<Connect to ip:port.
                                        *@param[in] ip IP address to connect to.
                                        *@param[in] port Port to connect to.*/
            void listen(const int backlog=5); /*!<Starts listening.
                                               *@param[in] backlog [default=5]
                                               Specifies maximum length of connection
                                               queue.*/
            void accept(); /*!<Accepts first connection in queue.*/
            string recv(); /*!<[blocking] Reads from client socket. Assumes a connection has been accepted.*/
            void send(string msg); /*!<[blocking] Writes to client socket. Assumes a connection has been accepted.*/
            void close(); /*!<Closes client socket.*/
            int get_server_fd(); /*!<Get server socket descriptor.
                                  *\return Server socket descriptor.*/
            int get_client_fd(); /*!<Get client socket descriptor.
                                  *\return Client socket descriptor.*/
        private:
            static const size_t size = 30000;
            bool is_server;
            bool is_client;
            char *buf;
            struct sockaddr_in server_addr;
            struct sockaddr_in client_addr;
        protected:
            int server_fd; /*!<Server file descriptor.*/
            int client_fd; /*!<Client file descriptor.*/
    };

    /**
     * Exception class for internal socket errors
    */
    class socket_error : public runtime_error {
        public:
            socket_error(const std::string& msg); /*!<Class constructor.*/
    };
}

#endif
