#ifndef _HTTPSERVER_H
#define _HTTPSERVER_H

#include <gollum2411.h>
#include <memory>

using std::string;
using std::shared_ptr;

/**
 * Simple HTTP server
*/

class HTTPServer : protected gollum2411::Socket{
    public:
        HTTPServer(); /*!<Class constructor.*/
        ~HTTPServer(); /*!<Class destructor.*/
        /**
         * Starts server.
         * @param[in] port Port to bind to.
        */
        void start_server(const int port);
        void serve(); /*!<Serves a client.
                       This method will serve only one connection,
                       so it is recommended to call this from within
                       a loop. Note that this will only serve files
                       located in the same directory from where the
                       binary is invoked.*/
    private:
        string get_file_to_serve(string &data);
        void process_request(string &data);
        void show_default();
        void send_404();
        shared_ptr<Socket> client_response;
};

#endif //_HTTPSERVER_H
