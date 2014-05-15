#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sstream>
#include <memory>

#include <gollum2411.h>

using std::stringstream;
using std::shared_ptr;
using std::string;
using std::cout;
using std::endl;
using std::cerr;

using gollum2411::read_from_ssl;
using gollum2411::write_to_ssl;
using gollum2411::socket_error;
using gollum2411::NEWLINE;
using gollum2411::Socket;

void init_ctx(SSL_CTX **ctx)
{
    const SSL_METHOD *method;
    SSL_library_init();
    if(ctx == NULL){
        cerr << "NULL SSL_CTX ptr" << endl;
        exit(1);
    }

    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    method = SSLv23_client_method();
    *ctx = SSL_CTX_new(method);
    if (*ctx == NULL)
    {
        ERR_print_errors_fp(stderr);
        return;
    }

    SSL_CTX_set_options(*ctx, SSL_OP_NO_SSLv2);
}

void load_certs(SSL_CTX *ctx, string cert_file, string key_file, string CA)
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

    SSL_CTX_load_verify_locations(ctx, CA.c_str(), ".");
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_verify_depth(ctx, 1);

}

void get_file(SSL *ssl, string file){
    size_t filesize = 0;
    stringstream buf;
    string response;
    char readbytes[BUFSIZE] = {0};

    //Place request
    buf << "Get: " << file << NEWLINE;
    write_to_ssl(ssl, buf.str());
    buf.str("");

    //Check if server claims existence of file
    response = read_from_ssl(ssl);
    if(response.find("NotFound") != string::npos){
        // TODO: Create custom exception type
        cerr << "File is not in server" << endl;
    }else if(response.find("OK") != string::npos){
        debug("File found\n");
    }else{
        throw socket_error("filetransfer protocol mismatch");
    }

    //Get size of file
    response = read_from_ssl(ssl);
    if(response.find("Size:") == string::npos){
        throw socket_error("filetransfer protocol mismatch");
    }

    response = slice(response, response.find(":") + 1, response.length());
    response = remove_newline(response);

    filesize = atoi(response.c_str());
    debug("file size = %u\n", filesize);

    buf.str("");
    buf << "OK" << NEWLINE;
    write_to_ssl(ssl, buf.str());

    FILE *f = fopen(basepath(file).c_str(), "w");
    if(!f){
        throw socket_error("error opening file for writing");
    }

    debug("Getting file...\n");
    while(SSL_read(ssl, (void*)readbytes, BUFSIZE) > 0){
        fwrite(readbytes, 1, BUFSIZE, f);
        bzero(readbytes, BUFSIZE);
    }
    debug("Done\n");
    fclose(f);
}

int main(int argc, char *argv[]){
    if(argc != 7){
        cerr << "Usage: " << argv[0] << " <ip> <port> <cert> <key> <CA> <file_to_get>" << endl;
	return -1;
    }
    
    string ip = argv[1];
    const int port = atoi(argv[2]);
    string cert = argv[3];
    string key = argv[4];
    string CA = argv[5];
    string file_to_get = argv[6];

    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    init_ctx(&ctx);
    if(ctx==NULL){
        cerr << "init_ctx" << endl;
        return -1;
    }
    
    load_certs(ctx, cert, key, CA);

    Socket sock(PF_INET, SOCK_STREAM, 0);
    sock.connect(ip.c_str(), port);

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock.get_sockfd());

    if(SSL_connect(ssl) == -1){
        cerr << "SSL connect failed: " << endl;
        ERR_print_errors_fp(stderr);
    }else{
        debug("Successful connection\n");
        debug("Cipher: %s\n", SSL_get_cipher(ssl));
        debug("Getting file: %s\n", file_to_get.c_str());
        get_file(ssl, file_to_get);
    }
    return 0;

}
