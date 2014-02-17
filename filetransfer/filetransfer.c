#include <utils.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LISTENER_PORT 7777
#define MAXBUF 256

int transfer_file(int client, const char *file_to_tranfer){
    int file = open(file_to_tranfer, O_RDONLY);
    int bytes_read = 0;
    int total_bytes = 0;
    char buf[MAXBUF] = {};
    if(file == -1){
        debug("Failed opening file %s", file_to_tranfer);
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
    struct sockaddr_in client_addr;
    socklen_t client_len = 0;
    char message[] = "Send 'START' to start transfer...\n";
    char buf[MAXBUF] = {0};
    char *file_to_transfer = NULL;
    int count = 0;

    if(argc != 2){
        printerr("Usage: %s <file_to_serve>\n", argv[0]);
        exit(-1);
    }

    file_to_transfer = argv[1];
    printf("File to transfer: %s\n", file_to_transfer);

    ret_val = setup_listener_socket(&server, port);
    if(ret_val || !server){
        reporterr(ret_val, errno);
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
        if(count < 0){
            printerr("Error reading client socket\n");
            reporterr(count, errno);
            close(client);
            continue;
        }
        if(strstr(buf, "START") != NULL){
            printf("Starting transfer...\n");
            if(transfer_file(client, file_to_transfer)){
                debug("File transfer failed...\n");
            };
        }
        printf("File transfer completed\n");
        debug("Shutting down socket...\n");
        debug("Closing connection...\n");
        close(client);
    }
}
