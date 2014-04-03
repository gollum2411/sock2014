#include <gollum2411.h>
#include <discover.h>
#include <fileserver.h>
#include <fileclient.h>
#include <thread>
#include <sstream>
#include <vector>
#include <stdexcept>

using std::string;
using std::endl;
using std::cout;
using std::cerr;
using std::stringstream;
using std::vector;
using std::out_of_range;
using gollum2411::Socket;


const char args_help[] = \
"Usage: filediscover command [options]\n\
\n\
Commands:\n\
    start-discover      Starts discovery server.\n\
    discover            Searches for other servers.\n\
    start-fileserver    Starts file server.\n\
    listfiles <server_ip> <port>\n\
                        Requests file list from server.\n\
    getfile <remote_filename> <server_ip> <port>\n\
                        Gets file from remote client.\n\
\n\
Options\n\
    -t <timeout>    Timeout for discovery operations.\n\
    -p <tcp port>   TCP port to listen on. Defaults to 4444.\n\
    -u <udp port>   UDP port to listen on. Defaults to 4444.";

void print_help(){
    cout << args_help << endl;
    return;
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

int main(int argc, char *argv[]){
    int timeout = 5;
    int c = 0;
    int udp_port = 4444;
    int tcp_port = 4444;
    stringstream arg;
    string command;
    vector<string> posargs;

    if(argc < 2){
        print_help();
        return -1;
    }

    while((c = getopt(argc, argv, "t:u:p:")) != -1){
        switch(c){
            case 't':
                arg << optarg;
                arg >> timeout;
                if(!is_number(arg.str())){
                    cerr << "Option -t expects a positive integer" << endl;
                    return -1;
                }
                arg.str("");
                arg.clear();
                break;
            case 'u':
                arg << optarg;
                arg >> udp_port;
                if(!is_number(arg.str())){
                    cerr << "Option -u expects a positive integer" << endl;
                    return -1;
                }
                arg.str("");
                arg.clear();
                break;
            case 'p':
                arg << optarg;
                arg >> tcp_port;
                if(!is_number(arg.str())){
                    cerr << "Option -p expects a positive integer" << endl;
                    return -1;
                }
                arg.str("");
                arg.clear();
                break;
            case 'h':
                print_help();
                return 0;
            case '?':
                print_help();
                return -1;
            default:
                print_help();
                return -1;
        }
    }
    if(optind >= argc){
        cerr << "Missing command" << endl;
        print_help();
        return -1;
    }

    for(int opt = optind; opt!=argc; ++opt){
        posargs.push_back(argv[opt]);
    }

    for(auto &cmd : posargs){
        if(cmd == "help"){
            print_help();
            return 0;
        }
    }

    command = posargs[0];
    arg.str("");
    arg.clear();

    if(command == "help"){
        print_help();
        return 0;
    }else if(command == "start-discover"){
        return start_discover_server(udp_port, tcp_port);
    }else if(command == "discover"){
        return look_for_servers(udp_port, timeout);
    }else if(command == "start-fileserver"){
        return start_fileserver(tcp_port);
    }else if(command == "listfiles"){
        string ip;
        int port = 0;
        try{
            ip = posargs.at(1);
            arg.str(posargs.at(2));
            if(!is_number(arg.str())){
                cerr << "Port must be a positive integer" << endl;
                return -1;
            }
            arg >> port;
        }catch(out_of_range &out){
            cerr << "Usage: listfiles <server_ip> <port>" << endl;
            return -1;
        }

        try{
            get_filelist(ip, port);
        }catch(gollum2411::socket_error &se){
            cerr << "listfiles failed: " << se.what() << endl;
            return -1;
        }
    }else if(command == "getfile"){
        string ip;
        string filename;
        int port = 0;
        try{
            filename = posargs.at(1);
            ip = posargs.at(2);
            arg.str(posargs.at(3));
            if(!is_number(arg.str())){
                cerr << "Port must be a positive integer" << endl;
                return -1;
            }
            arg >> port;
        }catch(out_of_range &out){
            cerr << "Usage: getfile <remote_filename> <server_ip> <port>" << endl;
            return -1;
        }

        try{
            get_file(filename, ip, port);
        }catch(gollum2411::socket_error &se){
            cerr << "getfile failed: " << se.what() << endl;
            return -1;
        }
    }

    return 0;
}