#include <gollum2411.h>

using gollum2411::TCPSocket;

using std::cout;
using std::endl;

int main(){
    TCPSocket server;
    TCPSocket client;

    server.bind(8080);
    server.listen();
    std::string recvd;
    while(1){
        server.accept();
        recvd = server.recv();
        cout << recvd << endl;
        server.close();
    }
    return 0;
}
