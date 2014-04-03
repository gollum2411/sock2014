#include <iostream>
#include <fstream>
#include <sstream>

#include <fileclient.h>
#include <gollum2411.h>
#include <protstrings.h>
#include <common.h>

using std::cout;
using std::string;
using std::stringstream;
using std::ofstream;
using gollum2411::Socket;

void do_ping(Socket::ptr sock, string ip, int port){
    debug("Pinging %s:%d\n", ip.c_str(), port);
    sock->connect(ip.c_str(), port);
    sock->send(PING + NEWLINE + NEWLINE);
    string response = sock->recv();
    if(response.substr(0, PONG.length()) != PONG){
        throw(gollum2411::socket_error("No PONG received"));
    }
}

void get_filelist(string ip, int port){
    Socket::ptr sock(new Socket(AF_INET, SOCK_STREAM, 0));
    do_ping(sock, ip, port);
    stringstream command;
    command << FILELIST << NEWLINE << NEWLINE;
    sock->send(command.str());
    cout << sock->recv();
}

void get_file(std::string filename, std::string ip, int port){
    Socket::ptr sock(new Socket(AF_INET, SOCK_STREAM, 0));
    do_ping(sock, ip, port);
    stringstream command;
    command << GETFILE << NEWLINE;
    command << NOMBRE << filename << NEWLINE << NEWLINE;

    sock->send(command.str());

    //Get file info
    // ToDo:actually CARE about the info
    sock->recv();

    //Get file
    size_t bytes_read = 0;
    size_t total_bytes = 0;
    char buf[MAXBUF] = {0};

    int done = 0;
    ofstream outfile(filename ,std::ofstream::binary);
    while(!done){
        bytes_read = read(sock->get_sockfd(), buf, MAXBUF);
        debug("Read %ld bytes\n", bytes_read);
        total_bytes += bytes_read;
        debug("Total bytes read: %ld\n", total_bytes);
        if(bytes_read <= 0){
            break;
        }
        outfile.write(buf, bytes_read);
        bzero(buf, MAXBUF);
    }
    outfile.close();
}