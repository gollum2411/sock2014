#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <utils.h>
#include <server.h>

#define PORT 7777
#define MAXBUF 256

int send_file(const int client, const char *file_to_send){
    int file = open(file_to_send, O_RDONLY);
    int bytes_read = 0;
    int total_bytes = 0;
    char buf[MAXBUF] = {0};

    if(file == -1){
        debug("Failed opening file %s", file_to_send);
        reporterr(file, errno);
        return -1;
    }

    bzero(buf, MAXBUF);

    bytes_read = total_bytes = read(file, buf, MAXBUF-1);
    if(bytes_read == 0 || bytes_read == -1){
        printerr("Empty file\n");
        return -1;
    }
    while(bytes_read != 0 ){
        write(client, buf, bytes_read);
        bytes_read = read(file, buf, MAXBUF-1);
        if(bytes_read == -1){
            debug("Error reading...\n");
            reporterr(bytes_read, errno);
            close(file);
            return -1;
        }
        total_bytes += bytes_read;
    }
    return 0;
}

int send_file_size(int client, const char *file_to_send){
    FILE *f = fopen(file_to_send, "r");
    char buf[MAXBUF] = {0};
    if(!f){
        printerr("Failed opening file: %s\n", file_to_send);
        reporterr(-1, errno);
        return -1;
    }
    size_t size = 0;

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fclose(f);
    snprintf(buf, size, "%ld", size);
    write(client, buf, MAXBUF);
    return 0;
}

int expect_start(const uint32_t client){
    char buf[MAXBUF] = {0};
    int bytes = 0;
    bytes = read(client, buf, MAXBUF);
    if(bytes <= 0){
        printerr("read failed\n");
        reporterr(bytes, errno);
        return -1;
    }
    if(strstr(buf, "START") != NULL){
        return 0;
    }else{
        return -1;
    }
}

int serve_file(const char *file_to_serve){
    int ret_val = -1;
    int server = 0;
    int client = 0;
    uint32_t port = PORT;
    socklen_t socksize = 0;
    struct sockaddr_in client_addr;

    debug("Setting up listener socket\n");
    ret_val = setup_listener_socket(&server, port);
    if(ret_val || !server){
        reporterr(ret_val, errno);
        return -1;
    }

    printf("Server started\n");
    printf("Serving %s\n", file_to_serve);
    while(1){
        client = accept(server, (struct sockaddr*)&client_addr, &socksize);
        if(client <= 0){
            printerr("Failed accepting connection: ");
            reporterr(client, errno);
            close(server);
        }
        debug("Accepted client connection\n");
        if(expect_start(client)){
            debug("Expected START from client. Closing connection.\n");
            close(client);
            continue;
        }
        debug("Received SART signal\n");
        if(send_file_size(client, file_to_serve)){
            printerr("Failed sending size\n");
            close(client);
            continue;
        }
        debug("Sent size successfully\n");
        if(expect_start(client)){
            debug("Expected START from client. Closing connection.\n");
            close(client);
            continue;
        }
        debug("Received START. Initiating transfer.\n");
        if(send_file(client, file_to_serve)){
            printerr("Transfer failed\n");
            close(client);
            continue;
        }
        debug("Transfer succeeded");
        close(client);
    }
}
