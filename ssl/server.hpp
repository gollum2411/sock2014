#include <openssl/ssl.h>
#include <openssl/err.h>
#include <fstream>

#include <gollum2411.h>

class SSLServer{
    public:
        SSLServer(const int port, std::string cert_file, std::string key_file, std::string CA_file);
        ~SSLServer();
        void init_server();
        void serve_forever();
    private:
        void load_certs();
        void init_ctx();
        void serve_client(SSL *ssl);
        const int port;
        static const int max_bufsize = 1024;
        gollum2411::Socket::ptr sock;
        std::string cert_file;
        std::string key_file;
        std::string CA_file;
        SSL_CTX *ctx;
        char *buf;
};
