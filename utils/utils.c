#include <utils.h>
#define BACKLOG 5

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