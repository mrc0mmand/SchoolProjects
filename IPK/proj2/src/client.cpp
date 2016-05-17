#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <exception>
#include <stdexcept>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>

#define PROTO_NAME "IPK"
#define PROTO_VER "0.1"
#define BUFFER_LENGTH 512

using namespace std;

/**
 * @brief Error codes
 */
enum ec {
    E_OK = 0,   /**< Everything is ok */
    E_SETUP,    /**< Error in server setup */
    E_PARAM,    /**< Invalid parameters */
    E_OTHER,    /**< Other error */
    E_SOCK,     /**< Error during socket() call */
    E_ACCEPT,   /**< Error during accept() call */
    E_WRITE,    /**< Error during write() call */
    E_CMD       /**< Error during command processing */
};

/**
 * @brief Connect to remote host
 *
 * @param port Target port
 * @param host Target host
 * @param sd Valid pointer where final socket will be stored in
 *
 * @return Error codes from ec enum
 */
int client_setup(int port, const string &host, int *sd);

/**
 * @brief Handle communication between client and server related to
 *        given command
 *
 * @param sd Valid server socket descriptor
 * @param file File name to GET/PUT
 * @param cmd PUT or GET
 */
int process_command(int sd, const string &file, const string &cmd);

int main(int argc, char *argv[])
{
    int opt, sd;
    int ec = E_OK;
    int port = -1;
    string hostname, filename, command;

    while((opt = getopt(argc, argv, "h:p:d:u:")) != -1) {
        switch(opt) {
        case 'h':
            hostname = optarg;
            break;
        case 'p':
            try {
                char *ptr;
                // Try to convert string to int
                port = strtol(optarg, &ptr, 10);

                if(*ptr != '\0')
                    throw invalid_argument("not a number");

                // Check port range
                if(port < 1 || port > 65535)
                    throw range_error("out of range");
            } catch(const exception &e) {
                cerr << "Invalid port (" << e.what() << ")" << endl;
                exit(E_PARAM);
            }

            break;
        case 'd':
            if(!filename.empty()) {
                cerr << "Only one of -u/-d options can be specified "
                        "at the same time" << endl;
                exit(E_PARAM);
            }

            filename = optarg;
            command = "GET";
            break;
        case 'u':
            if(!filename.empty()) {
                cerr << "Only one of -u/-d options can be specified "
                        "at the same time" << endl;
                exit(E_PARAM);
            }

            filename = optarg;
            command = "PUT";
            break;
        case '?':
            cout << "Usage: " << argv[0] << " -h hostname -p port [-d|u] "
                    "filename" << endl;
            exit(1);
        default:
            cerr << "Unexpected error during getopt() call" << endl;
            abort();
        }
    }

    if(optind < argc || hostname.empty() || filename.empty() || port == -1) {
        cout << "test" << endl;
        cout << "Usage: " << argv[0] << " -h hostname -p port [-d|u] "
                "filename" << endl;
        exit(E_PARAM);
    }

    ec = client_setup(port, hostname, &sd);
    if(ec != E_OK) {
        cerr << "Client setup failed" << endl;
        exit(ec);
    }

    ec = process_command(sd, filename, command);
    if(ec != E_OK) {
        cerr << "An error has occured during command processing" << endl;
    }

    if(sd != -1)
        close(sd);

    return ec;
}

int client_setup(int port, const string &host, int *sd)
{
    int rc;
    struct sockaddr_in server_addr;
    struct hostent *hostp;

    *sd = socket(AF_INET, SOCK_STREAM, 0);
    if(*sd < 0) {
        perror("socket() failed");
        exit(E_SOCK);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(host.c_str());

    // Server string was not an IP address
    if(server_addr.sin_addr.s_addr == INADDR_NONE) {
        hostp = gethostbyname(host.c_str());
        if(hostp == NULL) {
            cerr << "Unknown host: " << host << endl;
            return E_SETUP;
        }

        memcpy(&server_addr.sin_addr, hostp->h_addr_list[0],
               sizeof(server_addr.sin_addr));
    }

    rc = connect(*sd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(rc < 0) {
        perror("connect() failed");
        return E_SETUP;
    }

    return E_OK;
}

int process_command(int sd, const string &file, const string &cmd)
{
    int rc, status = -1;
    char buffer[BUFFER_LENGTH] = { 0, };
    string::size_type idx;
    string request, response, message;
    string confirm = "READY\r\n\r\n";
    stringstream ss;

    ss << PROTO_NAME << " " << PROTO_VER << " " << cmd << " " << file
       << "\r\n\r\n";

    request = ss.str();

    rc = write(sd, request.c_str(), request.size());
    if(rc < 0) {
        perror("send() failed");
        return E_WRITE;
    }

    while((rc = read(sd, buffer, BUFFER_LENGTH)) > 0) {
        response.append(buffer, rc);
        if((idx = response.find("\r\n\r\n")) != string::npos) {
            response.erase(idx);
            break;
        }
    }

    ss.str(response);
    ss >> status >> message;

    if(status != 0) {
        cerr << "Can't " << cmd << " file " << file << endl
             << "Error: " << status << ": " << message << endl;
        return E_CMD;
    }

    if(cmd == "PUT") {
        cout << "Uploading file '" << file << "'" << endl;
        ifstream in;
        string sync;
        try {
            in.open(file, ios::in|ios::binary);
            if(!in.is_open()) {
                perror("open() failed");
                throw runtime_error("unable to open file '" + file + "'");
            }
        } catch(const exception &e) {
            cerr << "Error: " << e.what() << endl;
            return E_CMD;
        }

        // Synchronize client and server
        rc = write(sd, confirm.c_str(), confirm.size());
        if(rc < 0) {
            perror("write() failed");
            return E_WRITE;
        }

        cout << "Waiting for server" << endl;
        while(in.good()) {
            in.read(buffer, BUFFER_LENGTH);
            rc = write(sd, buffer, in.gcount());
            if(rc < 0) {
                perror("write() failed");
                return E_WRITE;
            }
        }

        cout << "File uploaded" << endl;
        in.close();
    } else if(cmd == "GET") {
        cout << "Downloading file '" << file << "'" << endl;
        ofstream out;
        try {
            out.open(file, ios::out|ios::binary);
            if(!out.is_open()) {
                perror("open() failed");
                throw runtime_error("unable to open file '" + file + "'");
            }
        } catch(const exception &e) {
            cerr << "Error: " << e.what() << endl;
            return E_CMD;
        }

        // Synchronize client and server
        rc = write(sd, confirm.c_str(), confirm.size());
        if(rc < 0) {
            perror("write() failed");
            return E_WRITE;
        }

        cout << "Waiting for server" << endl;
        while((rc = read(sd, buffer, BUFFER_LENGTH)) > 0) {
            out.write(buffer, rc);
        }

        cout << "File downloaded" << endl;
        out.close();
    }

    return E_OK;
}
