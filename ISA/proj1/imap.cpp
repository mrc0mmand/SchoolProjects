#include <iostream>
#include <sstream>
#include <fstream>
#include <exception>
#include <unistd.h>
#include <cstring>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "imap.hpp"
#include "tls.hpp"
#include "utils.hpp"

int imap_connect(const config_data_t *config, connection_data_t *conn)
{
    int rc;
    struct sockaddr_in server_addr;
    struct hostent *hostp;
    char buf[64] = { 0, };
    std::string response;

    conn->sd = socket(AF_INET, SOCK_STREAM, 0);
    if(conn->sd < 0) {
        perror("socket() failed");
        exit(E_SOCK);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(config->port);
    server_addr.sin_addr.s_addr = inet_addr(config->server_addr.c_str());

    if(server_addr.sin_addr.s_addr == INADDR_NONE) {
        hostp = gethostbyname(config->server_addr.c_str());
        if(hostp == NULL) {
            std::cerr << "Unknown host: " << config->server_addr << std::endl;
            return E_SETUP;
        }

        memcpy(&server_addr.sin_addr, hostp->h_addr_list[0],
                sizeof(server_addr.sin_addr));
    }

    rc = connect(conn->sd, (struct sockaddr *)&server_addr,
                    sizeof(server_addr));
    if(rc < 0) {
        perror("connect() failed");
        return E_SETUP;
    }

    if(config->tls) {
        rc = tls_upgrade_connection(conn, config->cert_file, config->cert_dir);
        if(rc != E_OK)
            return E_SETUP;
    }

    while((rc = socket_read(conn, buf, 64)) > 0) {
        response.append(buf, rc);
        if(response.find_first_of("\r\n") != std::string::npos)
            break;
    }

    if(rc < 0) {
        perror("socket_read() failed");
        return E_SOCK;
    }

    if(response.compare(0, 4, "* OK") != 0) {
        std::cerr << "Server error" << std::endl;
        return E_SETUP;
    }

    return E_OK;
}

int imap_login(connection_data_t *conn, const std::string &user,
                const std::string &pass)
{
    unsigned int req_id = conn->cnt++;
    std::stringstream ss;
    std::string req;

    ss << req_id << " LOGIN " << user << " " << pass << "\r\n";
    req = ss.str();

    if(imap_exec_command(conn, req, req_id) != E_OK){
        return E_CMD;
    }

    return E_OK;
}

int imap_exec_command(connection_data_t *conn, const std::string &command,
                        unsigned int req_id, std::string *response)
{
    std::string resp;
    char buf[512] = { 0, };
    int rc;

    rc = socket_write(conn, (char*)command.c_str(), command.size());
    if(rc <= 0) {
        perror("socket_write() failed");
        return E_SOCK;
    }

    while((rc = socket_read(conn, buf, 512)) > 0) {
        resp.append(buf, rc);
        if(resp.find_first_of("\r\n") != std::string::npos)
            break;
    }

    if(rc < 0) {
        perror("socket_read() failed");
        return E_SOCK;
    }

    if(response != NULL)
        *response = resp;

    return imap_status_check(req_id, resp);
}

int imap_status_check(unsigned int req_id, std::string &response)
{
    unsigned int id = 0;
    std::string status;

    std::istringstream iss(response);
    std::string line;

    while(std::getline(iss, line)) {
        if(line[0] != '*') {
            iss.str(line);
            iss >> id >> status;
            // Possible statuses: OK, NO (server error), BAD (client error)
            if(id == req_id && status == "OK")
                return E_OK;

            break;
        }
    }

    return E_CMD;
}

int imap_select_mailbox(connection_data_t *conn, const std::string &mailbox)
{
    unsigned int req_id = conn->cnt++;
    std::stringstream ss;
    std::string response;
    std::string line;
    std::string req;
    int mail_count = -1;

    ss << req_id << " SELECT " << mailbox << "\r\n";
    req = ss.str();

    if(imap_exec_command(conn, req, req_id, &response) != E_OK){
        return E_CMD;
    }

    ss.clear();
    ss.str(response);
    while(std::getline(ss, line)) {
        if(sscanf(line.c_str(), "* %d EXISTS", &mail_count) == 1)
            break;
    }

    if(mail_count == -1) {
        std::cerr << "Couldn't get mail count" << std::endl;
        return E_CMD;
    }

    conn->mail_count = mail_count;

    return E_OK;
}

int imap_download_mail(connection_data_t *conn, int mail_id,
                        const std::string &dir, bool header_only)
{
    bool end_of_body = false;
    unsigned int req_id = conn->cnt++;
    int rc;
    int to_read = 0;
    int mail_size = 0;
    const int buf_size = 512;
    char buf[buf_size] = { 0, };
    std::stringstream os;
    std::string req;
    std::string response;
    std::string body;
    std::string line;
    std::string filename;
    std::ofstream out;

    req_id = conn->cnt++;
    os << req_id << " FETCH " << mail_id
       << ((header_only) ? " BODY[HEADER]" : " BODY[]") << "\r\n";
    req = os.str();

    rc = socket_write(conn, (char*)req.c_str(), req.size());
    if(rc <= 0) {
        perror("socket_write() failed");
        return E_SOCK;
    }

    // Get mail size
    while((rc = socket_read(conn, buf, 1)) > 0) {
        response.append(buf, rc);
        if(response.find("\r\n") != std::string::npos) {
            if(sscanf(response.c_str(), "* %*[0-9] FETCH %*[^{] {%d}",
                        &mail_size) != 1) {
                std::cerr << "Couldn't get mail size" << std::endl;
                return E_CMD;
            }

            break;
        }
    }

    if(mail_size <= 0) {
        std::cerr << "Invalid mail size" << std::endl;
        return E_CMD;
    }

    // Read mail content (header + body, if requested)
    to_read = (mail_size < buf_size) ? mail_size : buf_size;
    while((rc = socket_read(conn, buf, to_read)) > 0) {
        body.append(buf, rc);
        mail_size -= rc;
        to_read = (mail_size < buf_size) ? mail_size : buf_size;
    }

    // Read mail terminator (right parenthesis) to make sure, we got everything
    // Also, read the command status message
    response.clear();
    while((rc = socket_read(conn, buf, 1)) > 0) {
        response.append(buf, rc);
        if(response.find("\r\n") != std::string::npos) {
            if(!end_of_body) {
                if(response.find(")") == std::string::npos) {
                    std::cerr << "Incomplete response" << std::endl;
                    return E_CMD;
                }
                response.clear();
                end_of_body = true;
            } else {
                break;
            }
        }
    }

    if(rc < 0) {
        perror("socket_read() failed");
        return E_SOCK;
    }

    if(imap_status_check(req_id, response) != E_OK) {
        std::cerr << "Couldn't download mail body" << std::endl;
        return E_CMD;
    }

    filename = "mail-" + std::to_string(mail_id);
    out.open(dir + filename);
    if(!out.is_open()) {
        std::cerr << "Couldn't open file " << (dir + filename) << std::endl;
        return E_CMD;
    }

    out << body;
    out.close();

    return E_OK;
}

int imap_download_all(connection_data_t *conn, const config_data_t *config)
{
    int ec = E_OK;

    if(conn->mail_count == -1) {
        std::cerr << "Invalid/uninitialized mail count" << std::endl;
    }

    // First UID: 1
    for(int i = 1; i <= conn->mail_count; i++) {
        if(imap_download_mail(conn, i, config->out_dir, config->header_only)
                != E_OK) {
            std::cerr << "ERROR: Download failed" << std::endl;
            ec = E_DOWNLOAD;
        }
    }

    if(ec == E_OK)
        std::cerr << "Downloaded " << conn->mail_count
                  << ((config->header_only) ? " message headers" :
                    " messages") << " from mailbox " << config->mailbox
                  << std::endl;

    return ec;
}

int imap_download_new(connection_data_t *conn, const config_data_t *config)
{
    int ec = E_CMD;
    int mail_id;
    int mail_count = 0;
    unsigned int req_id = conn->cnt++;
    std::stringstream ss;
    std::string search_tag = "* SEARCH";
    std::string req;
    std::string response;
    std::string line;
    std::string item;

    ss << req_id << " SEARCH UNSEEN" << "\r\n";
    req = ss.str();

    if(imap_exec_command(conn, req, req_id, &response) != E_OK){
        return E_CMD;
    }

    ss.clear();
    ss.str(response);
    while(std::getline(ss, line)) {
        if(line.compare(0, search_tag.size(), search_tag) == 0) {
            ec = E_OK;
            ss.clear();
            ss.str(line);
            // Skip the search tag
            ss >> item >> item;
            // Parse mail UIDs
            while(ss >> item) {
                try {
                    mail_id = std::stoi(item);
                } catch(std::exception &e) {
                    std::cerr << "Invalid mail ID: " << item << std::endl;
                    return E_CMD;
                }

                if(imap_download_mail(conn, mail_id, config->out_dir,
                            config->header_only) != E_OK) {
                    std::cerr << "ERROR: Download failed" << std::endl;
                    ec = E_DOWNLOAD;
                }

                mail_count++;
            }

            break;
        }
    }

    if(ec != E_OK) {
        std::cerr << "Incorrect server's response" << std::endl;
    } else {
        std::cerr << "Downloaded " << mail_count
                  << ((config->header_only) ? " new message headers" :
                    " new messages") << " from mailbox " << config->mailbox
                  << std::endl;
    }

    return ec;
}
