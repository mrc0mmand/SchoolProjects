#ifndef __UTILS_H_INCLUDED
#define __UTILS_H_INCLUDED

#include <string>
#include <openssl/ssl.h>

/**
 * @brief Current configuration
 */
typedef struct {
    bool            tls = false;            /**< Enable TLS/SSL */
    bool            new_only = false;       /**< Download only new messages */
    bool            header_only = false;    /**< Download only headers */
    int             port = -1;              /**< Connection port */
    std::string     server_addr;            /**< Server address */
    std::string     cert_dir = "/etc/ssl/certs";    /**< Cert directory */
    std::string     cert_file;              /**< Cert file */
    std::string     auth_file;              /**< Credentials file */
    std::string     mailbox = "INBOX";      /**< Mailbox name */
    std::string     out_dir;                /**< Output directory */
} config_data_t;

/**
 * @brief Connection data
 */
typedef struct {
    bool tls = false;       /**< TLS state (enabled/disabled) */
    unsigned int cnt = 0;   /**< Request number */
    int mail_count = -1;    /**< Mail count for currently selected mailbox */
    int sd = -1;            /**< Socket descriptor */
    SSL *tlsd = NULL;       /**< OpenSSL socket descriptor */
} connection_data_t;

enum ec {
    E_OK = 0,       /**< Everything is ok */
    E_SETUP,        /**< Error in client setup */
    E_PARAM,        /**< Invalid parameters */
    E_TLS,          /**< Error in TLS setup */
    E_FILE,         /**< Error in file operation */
    E_DOWNLOAD,     /**< Error during mail download */
    E_SOCK,         /**< Error during socket() call */
    E_CMD           /**< Error during command processing */
};

/**
 * @brief Read credentials file and save parsed username & password into
 *        the respective arguments
 * @param file Credentials file name
 * @param user Destination argument for parsed username
 * @param password Destination argument for parsed password
 *
 * @return E_OK on success, E_FILE otherwise
 */
int read_creds_file(const std::string &file, std::string &user,
                    std::string &password);

/**
 * @brief Read data from specified (non)-SSL/TLS socket
 * @details This function serves as a wrapper around read() and SSL_read()
 *
 * @param conn Valid pointer to the structure with current connection data
 * @param buf Valid pointer where the read data will be stored in
 * @param nbyte Maximum number of bytes to read
 *
 * @return -1 on error, number of read bytes otherwise
 */
ssize_t socket_read(connection_data_t *conn, void *buf, size_t nbyte);

/**
 * @brief Write data to specified (non)-SSL/TLS socket
 * @details This function serves as a wrapper around write() and SSL_write()
 *
 * @param conn Valid pointer to the structure with current connection data
 * @param buf Data to write
 * @param nbyte Number of bytes to write
 *
 * @return -1 on error, number of written bytes otherwise
 */
ssize_t socket_write(connection_data_t *conn, void *buf, size_t nbyte);

#endif
