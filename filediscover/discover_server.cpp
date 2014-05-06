#include <thread>
#include <fstream>
#include <sstream>
#include <vector>

#include <gollum2411.h>
#include <discover.h>
#include <protstrings.h>
#include <common.h>

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::stringstream;
using std::vector;

using gollum2411::Socket;

int start_discover_server(int udp_port, int tcp_port){
    debug("Starting discover server\n");
    struct sockaddr_in response;
    Socket udp_socket(AF_INET, SOCK_DGRAM, 0);
    udp_socket.bind(udp_port);

    string name = get_node_name();
    if(name == ""){
        missing_cfg();
        return 1;
    }

    int file_count = get_file_count(SERVER_DIR);
    if(file_count == 0){
        cerr << "No files found in " << SERVER_DIR << endl;
        return 1;
    }

    stringstream msg;
    msg << HI << " " << name << NEWLINE;
    msg << PUERTO << " " << tcp_port << NEWLINE;
    msg << ARCHIVOS << " " << file_count << NEWLINE << NEWLINE;
    cout << "UDP server on port " << udp_port << "..." << endl;
    while(1){
        string resp = udp_socket.recvfrom(response);
        print_recv(response, resp);
        udp_socket.sendto(response, msg.str());
    }
    return 0;
}