#include <gollum2411.h>

using gollum2411::TCPSocket;

int main(){
    TCPSocket server;
    TCPSocket client;

    server.bind(7777);
    server.listen();
    std::string recvd;
    while(1){
        server.accept();
        server.send("jirijiri\n");
        recvd = server.recv();

        server.send("You just sent: " + recvd);
        server.close();
    }
    return 0;
}
