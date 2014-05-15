#include <memory>
#include <sstream>

#include <server.hpp>
#include <gollum2411.h>


using std::stringstream;
using std::shared_ptr;
using std::ifstream;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

using gollum2411::read_from_ssl;
using gollum2411::write_to_ssl;
using gollum2411::socket_error;
using gollum2411::NEWLINE;
using gollum2411::Socket;


SSLServer::SSLServer(const int _port, string _cert_file, string _key_file, string _CA_file) :
    port(_port), cert_file(_cert_file), key_file(_key_file),
    CA_file(_CA_file), ctx(NULL), buf(NULL)
{
    SSL_library_init();
    buf = new char[1024];
}

SSLServer::~SSLServer()
{
    if(ctx != NULL){
        SSL_CTX_free(ctx);
    }
    if(buf != NULL){
        delete[] buf;
    }
}

void SSLServer::init_server()
{
    debug("init_server\n");
    ctx = NULL;
    init_ctx();
    load_certs();

    SSL_CTX_load_verify_locations(ctx, CA_file.c_str(), ".");
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_verify_depth(ctx, 1);

    sock = shared_ptr<Socket>(new Socket(PF_INET, SOCK_STREAM, 0));
    sock->bind(port);
    sock->listen();
    debug("SSLServer listening on port %d\n", port);
}

void SSLServer::serve_forever()
{
    SSL *ssl = NULL;
    while(1){
        Socket::ptr s = sock->accept();
        ssl = SSL_new(ctx);
        if(!ssl){
            throw socket_error("SSL_new");
        }
        SSL_set_fd(ssl, s->get_sockfd());
        if(SSL_accept(ssl) == -1){
            cerr << "Prob'ly not :(" << endl;
            ERR_print_errors_fp(stderr);
            continue;
        }
        serve_client(ssl);
        close(SSL_get_fd(ssl));
    }
}

void SSLServer::load_certs()
{
    if(SSL_CTX_use_certificate_file(ctx, cert_file.c_str(), SSL_FILETYPE_PEM) <= 0){
        ERR_print_errors_fp(stderr);
        throw socket_error("SSL_CTX_user_certificate_file");
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, key_file.c_str(), SSL_FILETYPE_PEM) <= 0){
        ERR_print_errors_fp(stderr);
        throw socket_error("SSL_CTX_use_PrivateKey_file");
    }

    if (!SSL_CTX_check_private_key(ctx)){
        throw socket_error("private key-public cert mismatch");
    }
}

void SSLServer::init_ctx()
{
    const SSL_METHOD *method;
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    method = SSLv23_server_method();
    ctx = SSL_CTX_new(method);
    if (ctx == NULL)
    {
        ERR_print_errors_fp(stderr);
        throw socket_error("init_ctx");
    }
}

void SSLServer::serve_client(SSL *ssl)
{
    stringstream buf;
    string response;
    string file_to_get;
    response = read_from_ssl(ssl);

    if(response.find("Get:") == string::npos){
        throw socket_error("file tansfer protocol mismatch");
    }
    response = slice(response, response.find(":") + 2, response.length());

    //Commented for debugging
    file_to_get = response;
    //file_to_get = basepath(reponse);


    //Remove newlines
    if(file_to_get.find(NEWLINE) != string::npos){
        file_to_get = remove_newline(file_to_get);
    }

    debug("Client requests file: %s\n", response.c_str());
    ifstream is(file_to_get.c_str(), std::ifstream::binary);
    if(is.fail()){
        debug("Local file not found\n");
        buf << "NotFound" << NEWLINE;
        write_to_ssl(ssl, buf.str());
        return;

    }

    debug("Local file found\n");
    buf << "OK" << NEWLINE;
    write_to_ssl(ssl, buf.str());
    buf.str("");

    is.seekg (0, is.end);
    buf << "Size: " << is.tellg() << NEWLINE;
    is.seekg (0, is.beg);

    write_to_ssl(ssl, buf.str());

    //Assume client says "OK\r\n"
    (void)read_from_ssl(ssl);
    char bytes[BUFSIZE] = {0};
    while(is){
        is.read(bytes, BUFSIZE);
        SSL_write(ssl, bytes, BUFSIZE);
        bzero(bytes, BUFSIZE);
    }

    is.close();
    return;
}

int main(int argc, char *argv[])
{
    if(argc != 5){
        cerr << "Usage: " << argv[0] << " <port> <cert_file> <key_file> <CA_file>" << endl;
        return 1;
    }

    int port = atoi(argv[1]);
    string cert_file = string(argv[2]);
    string key_file = string (argv[3]);
    string CA_file = string(argv[4]);
    SSLServer server(port, cert_file, key_file, CA_file);
    server.init_server();
    server.serve_forever();
}
