#include <iostream>
#include <map>
#include <stdexcept>
#include <sstream>
#include <vector>

#include <fileserver.h>
#include <protstrings.h>
#include <errstrings.h>
#include <common.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>

using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::string;
using std::vector;
using std::stringstream;
using std::runtime_error;
using gollum2411::Socket;

map<string, string> parse_request(string msg){
    map<string, string> request_items;
    string original_msg = msg;
    if(msg.length() > MAX_REQ_SIZE){
        stringstream ss;
        ss << MALFORMED_TCP_REQUEST << " : "
           << "lenght exceeds " << MAX_REQ_SIZE;
        throw(runtime_error(ss.str()));
    }

    string::size_type idx = 0;
    string::size_type idx2 = 0;
    string cmd_args;
    string key;
    string value;

    idx = msg.find(NEWLINE);
    if(idx == string::npos){
        throw(runtime_error(MALFORMED_TCP_REQUEST + "\n" + original_msg));
    }
    request_items["command"] = slice(msg, 0, idx);

    //Special cases: FILELIST|PING has no args. Return
    string command = request_items["command"];
    if(command == FILELIST || command == PING){
        return request_items;
    }

    // skip over newline
    // e.g.:
    // GETFILE\r\nNombre:filename\r\n\r\n
    idx = idx + NEWLINE.length();

    // Get only the args portion of msg
    msg = slice(msg, idx, msg.length());

    if(command == GETFILE){
        idx = 0;
        idx2 = msg.find(":");
        if(idx2 == string::npos){
            throw(runtime_error(MALFORMED_TCP_REQUEST + "\n" + original_msg));
        }
        key = slice(msg, idx, idx2);
        // When comparing, remember to strip ":" from constants
        if(key != NOMBRE.substr(0, NOMBRE.length() - 1)){
            throw(runtime_error(MALFORMED_TCP_REQUEST + "\n" + original_msg));
        }
        //idx points to [0], idx2 points to ":"
        // Nombre:filename\r\n
        idx = msg.find(NEWLINE);
        value = slice(msg, idx2+1, idx);
        request_items[key] = value;
        return request_items;
    }

    /*
    //Skip over first \r\n which signals end of CMD
    idx2 = idx = idx+NEWLINE.length();

    for(idx = msg.find(":"), idx2 = msg.find(NEWLINE);
        idx!=string::npos || idx2!=string::npos;
        idx++, idx2++){
        key = slice(msg, idx+1, idx2);

        request_items[]
    }*/
    return request_items;
}

void answer_filelist(Socket::ptr sock){
    int file_count = get_file_count(SERVER_DIR);
    stringstream ss;
    vector<string> files = get_files(SERVER_DIR);

    ss << OK << " " << FILELIST << NEWLINE;
    ss << CANTIDAD << file_count << NEWLINE << NEWLINE;

    for(auto &f : files){
        ss << f << NEWLINE;
    }

    sock->send(ss.str());
}

void send_file(Socket::ptr sock, string file){
    stringstream ss;
    size_t bytes_written = 0;
    //ToDo: send REAL size and md5sum
    ss << SIZE << 1000 << NEWLINE;
    ss << MD5 << "ae4bc071534fb8d0ae7eb2e6dc638266";
    sock->send(ss.str());

    file = SERVER_DIR + "/" + file;

    int fd = open(file.c_str(), O_RDONLY);

    struct stat s;
    if(stat(file.c_str(), &s)){
        cerr << "Could not stat file: " << file << endl;
        return;
    }

    bytes_written = sendfile(sock->get_sockfd(), fd,
                            NULL, s.st_size);
    if(bytes_written != (size_t)s.st_size){
        printerr("sendfile failed\n");
        reporterr(bytes_written, errno);
    }
    ::close(fd);
}

int start_fileserver(int tcp_port){
    debug("Starting file server\n");
    Socket::ptr socket(new Socket(AF_INET, SOCK_STREAM, 0));
    socket->bind(tcp_port);
    socket->listen();
    map<string, string> response;
    string command;
    cout << "TCP server on port " << tcp_port << "..." << endl;
    while(1){
        Socket::ptr resp = socket->accept();
        string msg = resp->recv();
        response = parse_request(msg);
        command = response["command"];

        //Expect PING from client
        if(command == PING){
            resp->send(PONG + NEWLINE + NEWLINE);
        }else{
            cerr << "No PING received" << endl;
            continue;
        }

        // Expect actual command
        msg = resp->recv();
        response = parse_request(msg);
        command = response["command"];

        if(command == FILELIST){
            answer_filelist(resp);
        }else if(command == GETFILE){
            send_file(resp, response["Nombre"]);
        }

    }
    return 0;
}