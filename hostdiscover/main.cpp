//#include <gollum2411.h>
#include <iostream>
#include <sstream>
#include <unistd.h>

//using namespace gollu2411;
using namespace std;

string args_help = \
"Usage: hostdiscover OPTION [OPTARG]\n\
    -s              Serve file FILE\n\
    -c              Get file and write to OUTFILE\n\
    ";

void print_help(){
    cout << args_help << endl;
    return;
}

int main(int argc, char *argv[]){
    bool do_serve = false;
    bool do_client = false;
    int c = -1;
    int port = -1;

    while((c = getopt(argc, argv, "scp:")) != -1){
        switch(c){
            case 's':
                do_serve = true;
                break;
            case 'c':
                do_client = true;
                break;
            case 'p':
                istringstream(optarg) >> port;
                break;
            case '?':
                cout << args_help << endl;
                break;
        }
    }

    if(do_serve && do_client){
        cerr << "Can't specify both -s and -c" << endl;
        return -1;
    }
}