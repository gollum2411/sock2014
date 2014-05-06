#include <thread>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include <utils.h>
#include <gollum2411.h>
#include <discover.h>
#include <errstrings.h>
#include <protstrings.h>
#include <common.h>

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::stringstream;
using std::ifstream;
using std::runtime_error;

using gollum2411::Socket;

class NodeInfo{
public:
    NodeInfo(string _name, int _port, int _file_count) :
        name(_name), port(_port), file_count(_file_count){};
    string name;
    int port;
    int file_count;
};

NodeInfo get_node_info(string msg){
    string name;
    stringstream temp;
    int port = 0;
    int file_count = 0;

    /*
    msg should contain something like:

    Hi:gollum2411-filediscover\r\n
    Puerto:4444\r\n
    Archivos:4\r\n\r\n

    Slice string to grab server name, port and file_count
    */

    string::size_type idx = msg.find(" ");
    if(idx == string::npos){
        throw runtime_error(MALFORMED_GREETING + " : \n" + msg);
    }

    int idx2 = msg.find(NEWLINE);
    if(idx == string::npos){
        throw runtime_error(MALFORMED_GREETING + " : \n" + msg);
    }
    name = slice(msg, idx+1, idx2);
    debug("name=%s\n", name.c_str());

    idx = msg.find(":", idx+1);
    if(idx == string::npos){
        throw runtime_error(MALFORMED_GREETING + " : \n" + msg);
    }

    idx2 = msg.find(NEWLINE, idx2 + NEWLINE.length());
    if(idx == string::npos){
        throw runtime_error(MALFORMED_GREETING + " : \n" + msg);
    }
    temp << slice(msg, idx+2, idx2);
    temp >> port;
    if(port == 0){
        throw runtime_error(MALFORMED_GREETING + " : \n" + msg);
    }
    debug("port=%d\n", port);

    idx = msg.find(":", idx+1);
    if(idx == string::npos){
        throw runtime_error(MALFORMED_GREETING + " : \n" + msg);
    }

    idx2 = msg.find(NEWLINE, idx2 + NEWLINE.length());
    if(idx == string::npos){
        throw runtime_error(MALFORMED_GREETING + " : \n" + msg);
    }
    //reset temp
    temp.str("");
    temp.clear();
    temp << slice(msg, idx+2, idx2);
    temp >> file_count;
    if(file_count == 0){
        throw runtime_error(MALFORMED_GREETING + " : \n" + msg);
    }
    debug("file_count=%d\n", file_count);

    return NodeInfo(name, port, file_count);
}

void print_recv(struct sockaddr_in host, string msg){
    char buf[255] = {0};
    if(inet_ntop(AF_INET, &host.sin_addr, buf, sizeof(buf)) == NULL){
        cerr << "inet_ntop failed" << endl;
        return;
    }

    int port = ntohs(host.sin_port);

    cout << buf << ":" << port << endl;
    debug("Host sent: \n%s\n", msg.c_str());
}

void print_found(struct sockaddr_in host, string msg){
    char buf[255] = {0};
    if(inet_ntop(AF_INET, &host.sin_addr, buf, sizeof(buf)) == NULL){
        cerr << "inet_ntop failed" << endl;
        return;
    }

    NodeInfo node = get_node_info(msg);
    cout << "\t" << node.name << " : " << buf << " : " << node.port << endl;
    cout << "\t\t" << "Files available: " << node.file_count << endl;

    debug("Host sent: \n%s\n", msg.c_str());
}

string get_node_name(){
    ifstream config(CFG_FILE);
    string node_name = "";
    if(config.good()){
        config >> node_name;
    }
    config.close();
    return node_name;
}

void missing_cfg(){
    cerr << "Failed getting node name" << endl;
    cerr << "Make sure " << CFG_FILE << " is a valid and readable file" << endl;
}

int look_for_servers(string bdcast, int udp_port, int timeout){
    int sockopt = 1;
    int ret_val = -1;
    Socket socket(AF_INET, SOCK_DGRAM, 0);
    debug("Setting SO_BROADCAST\n");
    ret_val = setsockopt(socket.get_sockfd(), SOL_SOCKET, SO_BROADCAST,
                         (const char *) &sockopt, sizeof(sockopt));
    if(ret_val){
        cerr << "Failed setting SO_BROADCAST: " << errno << " : " << strerror(errno) << endl;
        return 1;
    }

    struct timeval stimeout;
    stimeout.tv_sec = timeout;
    stimeout.tv_usec = 0;
    debug("Setting SO_RCVTIMEO\n");
    ret_val = setsockopt(socket.get_sockfd(), SOL_SOCKET, SO_RCVTIMEO,
                         (char*)&stimeout, sizeof(stimeout));
    if(ret_val){
        cerr << "Failed setting SO_RCVTIMEO: " << errno << " : "<< strerror(errno) << endl;
        return 1;
    }

    string node_name = get_node_name();
    if(node_name == ""){
        missing_cfg();
        return 1;
    }

    stringstream msg;
    msg << HELLO_FROM << " " << node_name << NEWLINE << NEWLINE;

    socket.sendto(bdcast.c_str(), udp_port, msg.str());
    cout << "Available servers:" << endl;
    // Since SO_RCVTIMEO has been set, catch socket_error and return
    try{
        while(1){
            struct sockaddr_in addr;
            print_found(addr, socket.recvfrom(addr));
        }
    }catch(gollum2411::socket_error &se){
        //Assume timeout has been reached
        return 0;
    }

    return 0;
}
