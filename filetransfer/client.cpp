#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <utils.h>
#include <client.h>
#include <common.h>

#define IP_ADDR "127.0.0.1"

int send_start(const int server){
    char buf[MAXBUF] = {0};
    int bytes = 0;
    snprintf(buf, MAXBUF, "START");
    bytes = write(server, buf, MAXBUF);
    if(bytes <= 0){
        printerr("write failed\n");
        reporterr(bytes, errno);
        return -1;
    }
    return 0;
}

int expect_size(const int client, size_t *size){
    char buf[MAXBUF] = {0};
    int bytes = 0;

    if(!size){
        debug("NULL size_t*\n");
        return -1;
    }

    bytes = read(client, buf, MAXBUF);
    if(bytes <= 0){
        printerr("Read failed\n");
        reporterr(bytes, errno);
        return -1;
    }
    *size = atoi(buf);
    return 0;
}

int receive_file(const int server, int fd, size_t filesize){
    size_t bytes_read = 0;
    size_t total_bytes = 0;
    char buf[MAXBUF] = {0};

    int done = 0;
    while(!done){
        bytes_read = read(server, buf, MAXBUF);
        debug("Read %ld bytes\n", bytes_read);
        debug("Total bytes read: %ld\n", total_bytes);
        total_bytes += bytes_read;
        if(bytes_read <= 0){
            if(total_bytes == filesize){
                printf("Transfer succeeded\n");
                return 0;
            }else{
                printf("Transfer failed\n");
                return -1;
            }
        }
        write(fd, buf, MAXBUF);
        bzero(buf, MAXBUF);
    }
    return -1;
}

int get_file(const char *outfile){
    size_t size = 0;
    debug("Connecting to: %s:%ld\n", IP_ADDR, PORT);
    int server = connect_to(IP_ADDR, PORT);
    int ret_val = -1;
    if(server < 0){
        close(server);
        return -1;
    }

    int f = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if(f < 0){
        printerr("open failed\n");
        reporterr(f, errno);
        close(server);
        return -1;
    }

    debug("Sending START\n");
    ret_val = send_start(server);
    if(ret_val){
        printerr("Failure sending START\n");
        close(server);
        return -1;
    }

    debug("Reading size\n");
    ret_val = expect_size(server, &size);
    if(ret_val){
        printerr("Failed getting size\n");
        close(server);
        return -1;
    }
    debug("Got file size: %ld\n", size);

    debug("Sending START to initiate transfer\n");
    ret_val = send_start(server);
    if(ret_val){
        printerr("Failure sending START\n");
        close(server);
        return -1;
    }

    debug("Start receiving file\n");
    ret_val = receive_file(server, f, size);
    if(ret_val){
        close(server);
        return -1;
    }
    close(server);
    return 0;
}
