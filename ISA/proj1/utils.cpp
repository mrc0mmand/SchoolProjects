#include <iostream>
#include <string>
#include <fstream>
#include <limits>
#include <unistd.h>
#include "utils.hpp"

int read_creds_file(const std::string &file, std::string &user,
                    std::string &password)
{
    std::ifstream in(file.c_str());
    std::string option;
    std::string sep;
    int ec = E_OK;

    if(!in.is_open()) {
        std::cerr << "Couldn't open file " << file << std::endl;
        return E_FILE;
    }

    in >> option >> sep >> user;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if(option != "username" && sep != "=") {
        ec = E_FILE;
        goto end;
    }

    in >> option >> sep >> password;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if(option != "password" && sep != "=") {
        ec = E_FILE;
        goto end;
    }

end:
    if(ec != E_OK) {
        user.clear();
        password.clear();
        std::cerr << "Invalid credentials file format" << std::endl;
    }

    in.close();

    return ec;
}

ssize_t socket_read(connection_data_t *conn, void *buf, size_t nbyte)
{
    if(conn->tls) {
        return SSL_read(conn->tlsd, buf, nbyte);
    } else {
        return read(conn->sd, buf, nbyte);
    }
}

ssize_t socket_write(connection_data_t *conn, void *buf, size_t nbyte)
{
    if(conn->tls) {
        return SSL_write(conn->tlsd, buf, nbyte);
    } else {
        return write(conn->sd, buf, nbyte);
    }
}
