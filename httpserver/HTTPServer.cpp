#include <sys/sendfile.h>
#include <HTTPServer.h>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


using std::string;
using std::cout;
using std::endl;
using std::stringstream;

HTTPServer::HTTPServer() :
TCPSocket()
{

}

HTTPServer::~HTTPServer(){

}

string HTTPServer::get_file_to_serve(string &data){
    // grab GET request, assume first line
    size_t pos = data.find("\r\n");
    string request = data.substr(0, pos);
    //At this point we have "GET /file/to/serve.txt HTTP/1.1"
    //Grab /file/to/serve.txt
    size_t file_start = request.find("/");
    //size_t space2 = request.find(" ", file_start+1);
    string s2 = request.substr(file_start);
    return s2.substr(0, s2.find(" "));
}

void HTTPServer::send_404(){
    stringstream response;
    response << "HTTP/1.0 404 File not found" << endl;
    response << "Server: gollum2411-httpserver" << endl;
    response << "Content-Type: text/html" << endl;
    response << "Connection: close" << endl << endl;
    response << "<html><body>File requested not found :(\
                 </body></html>" << endl;
    this->send(response.str());
    return;
}

void HTTPServer::show_default(){
    string html_response = "<html><body>Type file in URL bar</body></html>";
    stringstream response;
    response << "HTTP/1.1 200 OK" << endl;
    response << "Content-Type: text/html; charset=utf-8" << endl;
    response << "Content-Length: " << html_response.length() << endl;
    response << "Connection: close" << endl << endl;
    response << html_response;
    cout << "Response:" << endl << response.str() << endl;
    this->send(response.str());
    return;
}

void HTTPServer::process_request(string &data){
    string file_to_serve = ".";
    file_to_serve += this->get_file_to_serve(data);
    stringstream response;
    string html_response;

    if(file_to_serve == "./"){
        this->show_default();
        return;
    }

    size_t bytes_written = 0;
    string basefile = file_to_serve.substr(file_to_serve.find_last_of("/")+1);
    cout << "Getting file: " << file_to_serve << endl;
    cout << "Basefile: " << basefile << endl;
    int fd = open(file_to_serve.c_str(), O_RDONLY);
    if(fd == -1){
        this->send_404();
        return;
    }
    struct stat s;
    if(stat(file_to_serve.c_str(), &s)){
        printerr("stat failed");
        reporterr(-1, errno);
        return;
    }
    response << "HTTP/1.1 200 OK" << endl;
    response << "Content-Type: application/force-download" << endl;
    response << "Content-Type: application/octet-stream" << endl;
    response << "Content-Type: application/download" << endl;
    response << "Content-Description: File Transfer" << endl;
    response << "Content-Disposition: attachment; filename=" << basefile << endl;
    response << "Content-Length: " << s.st_size << endl;
    response << "Content-Transfer-Encoding: binary" << endl << endl;
    this->send(response.str());
    bytes_written = sendfile(this->client_fd, fd,
                            NULL, s.st_size);
    if(bytes_written != (size_t)s.st_size){
        printerr("sendfile failed\n");
        reporterr(bytes_written, errno);
    }
    ::close(fd);
}

void HTTPServer::start_server(const int port){
    this->bind(port);
    this->listen();
}

void HTTPServer::serve(){

    string recvd;
    this->accept();

    int state=1;
    setsockopt(this->client_fd, IPPROTO_TCP, TCP_CORK,
               &state, sizeof(state));
    recvd = this->recv();
    this->process_request(recvd);
    this->close();
}
