#include <utils.h>
#include <netdb.h>
#include <dirent.h>

#include <iostream>

#define BACKLOG 5

using std::string;
using std::vector;

void printerr(const char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void _debug(const char* filename, int line, FILE *desc, const char* fmt, ...){
#ifdef DEBUG
    fprintf(desc, "%s : %d : ", filename, line);
    va_list args;
    va_start(args, fmt);
    vfprintf(desc, fmt, args);
    va_end(args);
#endif
}

void reporterr(int ret_val, int _errno){
    printerr("Error: %d : errno %d : %s\n", ret_val, _errno, strerror(_errno));
}

int setup_listener_socket(int *server_desc, const uint32_t port){
    int ret_val = -1;
    int server = 0;
    struct sockaddr_in server_addr;
    int sockopt = 0;

    if(!server_desc){
        printerr("NULL server_desc pointer\n");
        return -1;
    }

    debug("Setting server_addr to zeros\n");
    bzero(&server_addr, sizeof(struct sockaddr_in));

    debug("Initializing server_addr to port %d\n", port);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    server = socket(PF_INET, SOCK_STREAM, 0);
    if(server == -1){
        reporterr(ret_val, errno);
        return -1;    }
    debug("Getting socket: desc = %d\n", server);

    //Set SO_REUSEADDR to avoid errors
    ret_val = setsockopt(server, SOL_SOCKET, SO_REUSEADDR,
                        (const char *) &sockopt, sizeof(sockopt));
    if(ret_val){
        debug("setsockopt() failed\n");
        reporterr(ret_val, errno);
        return -1;
    }

    ret_val = bind(server, (const struct sockaddr*)&server_addr,
                   (socklen_t)sizeof(struct sockaddr_in));
    if(ret_val){
        reporterr(ret_val, errno);
        close(server);
        return -1;
    }
    debug("Socket bound...\n");

    ret_val = listen(server, BACKLOG);
    if(ret_val == -1){
        reporterr(ret_val, errno);
        close(server);
        return -1;
    }
    debug("Listening...\n");

    *server_desc = server;
    return 0;
}

/*
    int connect_to(const char *ip_addr, const uint32_t port)
    Connect to ip_addr:port. Returns socket file descriptor.
*/

int connect_to(const char *ip_addr, const uint32_t port){
    int sockfd = 0;
    int ret_val = -1;
    struct hostent *server = NULL;
    struct sockaddr_in server_addr;

    bzero((void*)&server_addr, sizeof(struct sockaddr_in));

    server = gethostbyname(ip_addr);
    if(!server){
        printerr("gethostbyname() failed\n");
        reporterr(-1, errno);
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        printerr("Failed opening socket\n");
        reporterr(sockfd, errno);
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    memcpy((void*)&server_addr.sin_addr.s_addr, (void*)server->h_addr, (size_t)server->h_length);
    ret_val = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in));
    if(ret_val == -1){
        printerr("Failed connecting\n");
        reporterr(ret_val, errno);
        return -1;
    }

    return sockfd;
}

std::string slice(std::string str, string::size_type start, string::size_type end){
    std::string retstr;
    for(string::size_type i = start; i < end; ++i){
        if(i == str.length()){
            retstr = "";
            break;
        }
        retstr+=str[i];
    }
    return retstr;
}

vector<string> get_files(string path){
    debug("Getting files\n");
    DIR *dir;
    struct dirent *ent;
    string f;
    vector<string> files;
    if ((dir = opendir (path.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            f = ent->d_name;
            if(f == "." || f == "..") continue; // Don't count . or ..
            files.push_back(ent->d_name);
        }
        closedir(dir);
    }
    return files;
}

int get_file_count(string path){
    vector<string> files = get_files(path);
    if(files.empty()){
        return 0;
    }
    else return (int)files.size();
}

string remove_newline(string s){
    if(s.find("\r\n") != string::npos)
        return slice(s, 0, s.find("\r\n"));
    else
        return s;
}

string basepath(string s){
    //Commented for debug only
    size_t idx = s.find_last_of("/\\");

    // get basename only
    if(idx != string::npos){
        return slice(s, idx+1, s.length());
    }else{
        return s;
    }

}
