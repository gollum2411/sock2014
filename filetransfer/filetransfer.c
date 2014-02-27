#include <utils.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <server.h>
#include <client.h>

const char args_help[] = \
"Usage: filetransfer OPTION [OPTARG]\n\
    -s FILE         Serve file FILE\n\
    -g OUTFILE      Get file and write to OUTFILE\n";

void print_help(){
    printf("%s", args_help);
}

int main(int argc, char *argv[]){
    int do_serve= 0;
    int do_get = 0;
    int c = 0;
    char *file_to_serve = NULL;
    char *outfile = NULL;

    if(argc < 2){
        print_help();
        exit(-1);
    }

    while((c = getopt(argc, argv, "s:g:")) != -1){
        switch(c){
            case 's':
                file_to_serve = optarg;
                do_serve = 1;
                break;
            case 'g':
                outfile = optarg;
                do_get = 1;
                break;
            case '?':
                print_help();
                exit(-1);
            default:
                print_help();
                exit(-1);
        }
    }

    if(do_get && do_serve){
        printerr("Can't specify -s and -g\n");
        exit(-1);
    }

    if(do_get){
        if(get_file(outfile)){
            printf("Failed getting file\n");
            return -1;
        }
    }

    if(do_serve){
        printf("Serving file %s\n", file_to_serve);
        serve_file(file_to_serve);
    }

    return 0;
}
