#include <iostream>

#include <HTTPServer.h>
#include <signal.h>

using std::cout;
using std::endl;

static bool do_cleanup = false;

void sighdl(int sig){
    do_cleanup = true;
}

int main(){
    HTTPServer httpserver;
    httpserver.start_server(8080);
    signal(SIGINT, sighdl);
    while(1){
        httpserver.serve();
        if(do_cleanup){
            cout << "Shutting down server" << endl;
            break;
        }
    }
    return 0;
}
