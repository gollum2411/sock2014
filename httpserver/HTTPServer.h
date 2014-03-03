#ifndef _HTTPSERVER_H
#define _HTTPSERVER_H

#include <gollum2411.h>

using std::string;

/**
 * Simple HTTP server
*/

class HTTPServer : protected gollum2411::TCPSocket{
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
                       a loop.*/
    private:
        string get_file_to_serve(string &data);
        void process_request(string &data);
        void show_default();
        void send_404();
};

#endif //_HTTPSERVER_H
