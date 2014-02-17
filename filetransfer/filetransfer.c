#include <utils.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FILE_TO_TRANSFER "/tmp/something.txt"
#define BACKLOG 5
#define LISTENER_PORT 7777
#define MAXBUF 256

int transfer_file(int client){
    int file = open(FILE_TO_TRANSFER, O_RDONLY);
    int bytes_read = 0;
    int total_bytes = 0;
    char buf[MAXBUF] = {};
    if(file == -1){
        debug("Failed opening file %s", FILE_TO_TRANSFER);
        reporterr(file, errno);
    }

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

int main(int argc, char *argv[]){
    int server = 0;
    int client = 0;
    int ret_val = -1;
    uint32_t port = LISTENER_PORT;
    struct sockaddr_in addr;
    struct sockaddr_in client_addr;
    socklen_t client_len = 0;
    char message[] = "Send 'START' to start transfer...\n";
    char buf[MAXBUF] = {0};
    int count = 0;

    printf("File to transfer must be located in %s\n", FILE_TO_TRANSFER);

    ret_val = get_bind_ipv4_server(&server, port);
    if(ret_val || !server){
        reporterr(ret_val, errno);
        exit(-1);
    }

    debug("Starting to listen...\n");
    ret_val = listen(server, BACKLOG);
    if(ret_val == -1){
        reporterr(ret_val, errno);
        close(server);
        exit(-1);
    }

    printf("Starting server on port %u\n", port);

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

        write(client, message, strlen(message));
        count = read(client, (void*)buf, MAXBUF-1);
        if(strstr(buf, "START") != NULL){
            printf("Starting transfer...\n");
            if(transfer_file(client)){
                debug("File transfer failed...\n");
            };
        }
        printf("File transfer completed\n");
        debug("Shutting down socket...\n");
        debug("Closing connection...\n");
        close(client);
    }
}
