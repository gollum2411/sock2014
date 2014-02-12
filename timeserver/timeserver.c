#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define BACKLOG 5

//TODO : add signal handler to properly shutdown server
//TODO : Move printerr, debug, and reporterr to common file

void printerr(const char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void debug(const char* fmt, ...){
#ifdef DEBUG
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
#endif
}

void reporterr(int ret_val, int _errno){
    printerr("Error: %d : errno %d : %s\n", ret_val, _errno, strerror(_errno));
}

int main(int argc, char *argv[]){
    uint32_t port = 0;
    int ret_val = -1;
    int server = 0;
    int client = 0;
    char currtime[256];
    socklen_t client_len = 0;
    time_t t;
    struct tm tm;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    if(argc != 2){
        printerr("Usage: %s <port_number>\n", argv[0]);
        exit(-1);
    }
    port = atoi(argv[1]);
    if(port < 1 || port > 65535){
        printerr("Port must be between 1 and 65535\n");
        exit(-1);
    }

    debug("Setting server_addr to zeros\n");
    bzero(&server_addr, sizeof(struct sockaddr_in));

    debug("Initializing server_addr. Listening on port %d\n", port);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    server = socket(AF_INET, SOCK_STREAM, 0);
    if(server == -1){
        reporterr(ret_val, errno);
        exit(-1);
    }
    debug("Getting socket: desc = %d\n", server);

    debug("Binding socket...\n");
    ret_val = bind(server, (const struct sockaddr*)&server_addr,
                   (socklen_t)sizeof(struct sockaddr_in));
    if(ret_val != 0){
        reporterr(ret_val, errno);
        close(server);
        exit(-1);
    }

    debug("Starting to listen...\n");
    ret_val = listen(server, BACKLOG);
    if(ret_val == -1){
        reporterr(ret_val, errno);
        close(server);
        exit(-1);
    }

    printf("Starting server on port %u...\n", port);

    while(1){
        debug("Setting client_addr to zeros\n");
        bzero(&client_addr, sizeof(struct sockaddr_in));
        debug("Waiting for connection...\n");
        client_len = sizeof(struct sockaddr_in);
        client = accept(server, (struct sockaddr*)&client_addr,
                        &client_len);
        if(client == -1){
            reporterr(client, errno);
            close(server);
            exit(-1);
        }
        debug("Writing to socket...\n");

        t = time(NULL);
        tm = *localtime(&t);

        sprintf(currtime, "%d-%d-%d %d:%d:%d\n", tm.tm_year + 1900,
                tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
                tm.tm_min, tm.tm_sec);

        write(client, currtime, strlen(currtime));
        debug("Shutting down socket...\n");
        debug("Closing connection...\n");
        close(client);
    }
}
