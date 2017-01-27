#ifndef __TLS_HPP_INCLUDED
#define __TLS_HPP_INCLUDED

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/x509v3.h>
#include "utils.hpp"

/**
 * @brief Initialize internal OpenSSL library structures
 */
void tls_init();

/**
 * @brief Cleanup internal OpenSSL library structures
 */
void tls_cleanup();

/**
 * @brief Shutdown TLS/SSL socket and free its memory
 *
 * @param tls Pointer to TLS/SSL socket
 */
void tls_shutdown(SSL *tls);

/**
 * @brief Upgrade non-SSL/TLS connection to SSL/TLS
 *
 * @param conn Connection data
 * @param certfile Path to a file with certificate(s) for a peer certificate
 *                 verification
 * @param certdir Path to a directory with certificates for a peer certificate
 *                verification
 *
 * @return E_OK on success, E_TLS otherwise
 */
int tls_upgrade_connection(connection_data_t *conn,
                           const std::string &certfile,
                           const std::string &certdir);

#endif
