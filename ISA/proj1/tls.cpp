#include <iostream>
#include "tls.hpp"

void tls_init()
{
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    SSL_load_error_strings();
    if(SSL_library_init() < 0) {
        std::cerr << "Couldn't initialize OpenSSL library" << std::endl;
        exit(E_TLS);
    }
}

void tls_cleanup()
{
    ERR_free_strings();
    EVP_cleanup();
}

void tls_shutdown(SSL *tls)
{
    if(tls != NULL)
        SSL_shutdown(tls);
    SSL_free(tls);
}

static int tls_verify_callback(int preverify_ok, X509_STORE_CTX *ctx)
{
    X509 *err_cert = NULL;
    char buff[256];

    if(preverify_ok != 1) {
        err_cert = X509_STORE_CTX_get_current_cert(ctx);
        X509_NAME_oneline(X509_get_subject_name(err_cert), buff, 256);
        std::cerr << "Couldn't verify certificate: " << buff << std::endl;
    }

    return preverify_ok;
}

int tls_upgrade_connection(connection_data_t *conn,
                           const std::string &certfile,
                           const std::string &certdir)
{
    SSL_CTX *ctx;
    const char *cafile = NULL;
    const char *capath = NULL;
    int rc = E_TLS;

    tls_init();

    ctx = SSL_CTX_new(SSLv23_client_method());
    if(ctx == NULL) {
        perror("SSL_CTX_new() failed");
        goto end;
    }

    cafile = certfile.empty() ? NULL : certfile.c_str();
    capath = certdir.empty() ? NULL : certdir.c_str();

    // SSLv3 is considered insecure, but it should be enabled as requested
    // by project assignment
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
    if(cafile != NULL || capath != NULL) {
        if(SSL_CTX_load_verify_locations(ctx, cafile, capath) != 1) {
            std::cerr << "Couldn't load certificates from specified "
                      << "location(s)" << std::endl;
            goto end;
        }
    } else {
        SSL_CTX_set_default_verify_paths(ctx);
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, tls_verify_callback);
    conn->tlsd = SSL_new(ctx);
    SSL_set_fd(conn->tlsd, conn->sd);
    if(SSL_connect(conn->tlsd) != 1) {
        std::cerr << "Couldn't upgrade given connection to TLS" << std::endl;
        goto end;
    }

    conn->tls = true;

    rc = SSL_get_verify_result(conn->tlsd);
    if(rc != X509_V_OK) {
        std::cerr << "Certificate verification failed " << std::endl;
        goto end;
    }

    rc = E_OK;

end:
    return rc;
}
