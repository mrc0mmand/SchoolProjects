#include <iostream>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "imap.hpp"
#include "tls.hpp"
#include "utils.hpp"

using namespace std;

/**
 * @brief Parse command line arguments
 *
 * @param argc Number of arguments
 * @param argv Array of string arguments
 * @param config Pointer to allocated destination configuration data structure
 *
 * @return E_OK on success, E_PARAM otherwise
 */
int parse_arguments(int argc, char *argv[], config_data_t *config);

/**
 * @brief Print contents of given config_data_t structure to the
 *        standard output
 *
 * @brief config Pointer to the config_data_t structure
 */
void dump_config(const config_data_t *config);

int main(int argc, char *argv[])
{
    int rc = E_OK;
    string username;
    string password;
    config_data_t config;
    connection_data_t conn;

    if(argc == 1) {
        cout << "Usage: " << argv[0] << " server [-p port] [-T [-c certfile] "
             << "[-C certdir]] [-n] [-h] [-a auth_file] [-b MAILBOX] "
             << "-o out_dir" << endl << endl
             << "  server\t\tserver IP/domain name" << endl
             << "  -p port\t\tserver port (default: " << IMAP_PORT << "/"
                << IMAPS_PORT << " IMAP/IMAPS)" << endl
             << "  -T\t\t\tturn on SSL/TLS" << endl
             << "  -c certfile\t\tfile with SSL/TLS certificate(s)" << endl
             << "  -C certdir\t\tdirectory with SSL/TLS certificates "
                << "(default: /etc/ssl/certs)" << endl
             << "  -n\t\t\tread only new/unread messages" << endl
             << "  -h\t\t\tdownload only email headers" << endl
             << "  -a auth_file\t\tfile with user credentials" << endl
             << "  -b MAILBOX\t\ttarget mailbox name (default: INBOX)" << endl
             << "  -o out_dir\t\toutput directory for downloaded messsages"
             << endl << endl
             << "Example auth_file:" << endl
             << "  username = user" << endl
             << "  password = pass" << endl;
        return E_OK;
    }

    rc = parse_arguments(argc, argv, &config);
    if(rc != E_OK)
        exit(rc);

    rc = read_creds_file(config.auth_file, username, password);
    if(rc != E_OK)
        exit(rc);

    rc = imap_connect(&config, &conn);
    if(rc != E_OK) {
        cerr << "Client setup failed" << endl;
        goto end;
    }

    rc = imap_login(&conn, username, password);
    if(rc != E_OK) {
        cerr << "Login failed" << endl;
        goto end;
    }

    rc = imap_select_mailbox(&conn, config.mailbox);
    if(rc != E_OK) {
        cerr << "Mailbox selection failed" << endl;
        goto end;
    }

    if(config.new_only)
        rc = imap_download_new(&conn, &config);
    else
        rc = imap_download_all(&conn, &config);

    if(rc != E_OK) {
        cerr << "Mail download failed" << endl;
        goto end;
    }

end:
    if(conn.tls) {
        tls_cleanup();
        tls_shutdown(conn.tlsd);
    }
    close(conn.sd);

    return rc;
}

int parse_arguments(int argc, char *argv[], config_data_t *config)
{
    int c;

    while((c = getopt(argc, argv, ":a:b:c:C:hno:p:T")) != -1) {
        switch(c) {
        case 'a':
            config->auth_file = optarg;
            break;
        case 'b':
            config->mailbox = optarg;
            break;
        case 'c':
            config->cert_file = optarg;
            break;
        case 'C':
            config->cert_dir = optarg;
            break;
        case 'h':
            config->header_only = true;
            break;
        case 'n':
            config->new_only = true;
            break;
        case 'o':
            config->out_dir = optarg;
            if(config->out_dir.find_last_of("/") != config->out_dir.size() - 1)
                config->out_dir.append("/");
            break;
        case 'p':
            try {
                char *ptr;
                config->port = strtol(optarg, &ptr, 10);

                if(ptr == nullptr || *ptr != '\0') {
                    throw invalid_argument("not a number");
                } else if(config->port < 1 || config->port > 65535) {
                    throw range_error("out of range");
                }

            } catch(const exception &e) {
                cerr << "Invalid port: " << e.what() << endl;
                return E_PARAM;
            }
            break;
        case 'T':
            config->tls = true;
            break;
        case ':':
            cerr << "Option -" << (char)optopt << " requires an operand"
                 << endl;
            return E_PARAM;
            break;
        case '?':
            cerr << "Unrecognized option: -" << (char)optopt << endl;
            return 1;
            break;
        default:
            cerr << "This should never happen." << endl;
            return 255;
        }
    }

    if(optind < argc) {
        config->server_addr = argv[optind++];
    } else {
        cerr << "Missing server address" << endl;
        return E_PARAM;
    }

    if(optind < argc) {
        cerr << "Unrecognized positional parameter: " << argv[optind] << endl;
        return E_PARAM;
    }

    if(config->out_dir.empty()) {
        cerr << "Missing output directory" << endl;
        return E_PARAM;
    }

    if(config->auth_file.empty()) {
        cerr << "Missing credentials file" << endl;
        return E_PARAM;
    }

    if(config->port == -1) {
        config->port = (config->tls) ? IMAPS_PORT : IMAP_PORT;
    }

    return E_OK;
}

void dump_config(const config_data_t *config)
{
    if(config == NULL)
        return;

    cout << "TLS\t\t" << ((config->tls) ? "yes" : "no") << endl
         << "New only:\t" << ((config->new_only) ? "yes" : "no") << endl
         << "Header only:\t" << ((config->header_only) ? "yes" : "no") << endl
         << "Port:\t\t" << config->port << endl
         << "Server addr:\t" << config->server_addr << endl
         << "Cert dir:\t" << config->cert_dir << endl
         << "Cert file:\t" << config->cert_file << endl
         << "Auth file:\t" << config->auth_file << endl
         << "Mailbox:\t" << config->mailbox << endl
         << "Out dir:\t" << config->out_dir << endl;
}
