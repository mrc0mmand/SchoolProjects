#ifndef __IMAP_H_INCLUDED
#define __IMAP_H_INCLUDED

#include <string>
#include "utils.hpp"

#define IMAP_PORT  143
#define IMAPS_PORT 993

/**
 * @brief Connect to an IMAP server
 *
 * @param config Current configuration data
 * @param conn Valid pointer to the connection data structure
 *
 * @return E_OK on success, EC from ec enum otherwise
 */
int imap_connect(const config_data_t *config, connection_data_t *conn);

/**
 * @brief Execute LOGIN command on an IMAP server
 *
 * @param conn Current connection data
 * @param user Username to login with
 * @param pass Password to login with
 *
 * @return E_OK on success, E_CMD otherwise
 */
int imap_login(connection_data_t *conn, const std::string &user,
                const std::string &pass);

/**
 * @brief Execute given command on an IMAP server
 *
 * @param conn Current connection data
 * @param command Command to execute
 * @param req_id Request ID
 * @param response If not NULL, it will contain server response
 *
 * @return E_OK on success, EC from ec enum otherwise
 */
int imap_exec_command(connection_data_t *conn, const std::string &command,
                        unsigned int req_id, std::string *response = NULL);

/**
 * @brief Check status of executed command
 *
 * @param req_id Expected request ID\
 * @param response Server response after command execution
 *
 * @return E_OK on success, E_CMD otherwise
 */
int imap_status_check(unsigned int req_id, std::string &response);

/**
 * @brief Set given mailbox as an active mailbox
 *
 * @param conn Current connection data
 * @param mailbox Mailbox name
 *
 * @return E_OK on success, E_CMD otherwise
 */
int imap_select_mailbox(connection_data_t *conn, const std::string &mailbox);

/**
 * @brief Download a mail with given UID from an IMAP server
 *
 * @param conn Currecnt connection data
 * @param mail_id UID of requested mail
 * @param dir Directory where the downloaded mail will be stored in
 *
 * @return E_OK on success, EC from ec enum otherwise
 */
int imap_download_mail(connection_data_t *conn, int mail_id,
                        const std::string &dir, bool header_only = false);

/**
 * @brief Download all mail according to the current configuration
 *
 * @param conn Current connection data
 * @param config Current configuration
 *
 * @return E_OK on success, E_DOWNLOAD otherwise
 */
int imap_download_all(connection_data_t *conn, const config_data_t *config);

/**
 * @brief Download all new mail according to the current configuration
 *
 * @param conn Current connection data
 * @param config Current configuration
 *
 * @return E_OK on success, E_DOWNLOAD otherwise
 */
int imap_download_new(connection_data_t *conn, const config_data_t *config);

#endif
