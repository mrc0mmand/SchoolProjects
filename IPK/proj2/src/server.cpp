#include <iostream>
#include <string>
#include <vector>
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
#include <unistd.h>
#include <signal.h>

#define PROTO_NAME "IPK"
#define CLIENT_QUEUE 10
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
};

/**
 * @brief Simple class for protocol error handling
 */
class ProtoErr {
public:
    /**
     * @brief Construct ProtoErr object and appropriate message
     *
     * @param ec Error code (@see enum p_ec)
     */
    ProtoErr(int ec) : error_code(ec) {
        ostringstream os;
        if(ec < 0 || (size_t)ec >= p_emsg.size())
            os << ec << " UNKNOWN_ERROR";
        else
            os << ec << " " << p_emsg[ec];

        os << "\r\n\r\n";
        message = os.str();
    }

    /**
     * @brief Return final messsage which consist of error code
     *        and error message
     *
     * @return CString with error message
     */
    const char *msg() { return message.c_str(); }

    /**
     * @brief Return length of error message (useful for eg. write())
     *
     * @return Error message length
     */
    size_t len() { return message.size(); }

    /**
     * @brief Send error message to given socket descriptor
     *
     * @param sd Valid socket descriptor
     *
     * @return E_OK on success E_WRITE otherwise
     */
    int send(int sd) {
        int rc;
        rc = write(sd, msg(), len());

        return (rc < 0) ? E_WRITE : E_OK;
    }

public:
    /**
     * @brief Protocol error codes
     */
    enum p_ec {
        PE_OK = 0,          /**< Everything is ok */
        PE_INVALID_PROTO,   /**< Invalid protocol name */
        PE_INVALID_VER,     /**< Invalid protocol version */
        PE_INVALID_CMD,     /**< Invalid command */
        PE_INVALID_FILE,    /**< Invalid filename */
        PE_GET_ERROR,       /**< Error during GET */
        PE_PUT_ERROR,       /**< Error during PUT */
        PE_ENUM_SIZE        /**< Placeholder for enum size */
    };

    /**
     * @brief Error messages for error codes in enum p_ec
     */
    vector<string> p_emsg = {
        "OK",
        "INVALID_PROTOCOL",
        "INVALID_VERSION",
        "INVALID_COMMAND",
        "INVALID_FILE",
        "GET_ERROR"
    };

private:
    int error_code;
    string message;
};

/**
 * @brief Wait for child and clear its resources
 *
 * @param sig_num Useless but necessary parameter for signal()
 */
void catch_child(int sig_num);

/**
 * @brief Left-trim all white characters from given string
 *
 * @param str String to trim
 *
 * @return Trimmed string
 */
string ltrim(const string &str);

/**
 * @brief Create server socket
 * 
 * @param port Port which server will listen on
 * @param sd Valid pointer where final socket will be stored in
 *
 * @return Error codes from ec enum
 */
int server_setup(int port, int *sd);

/**
 * @brief Listen for and handle clients
 * @details Wait on accept() and create a new process for each new client
 *
 * @param sd Valid socket descriptor which server listens on
 *
 * @return Error codes from ec enum
 */
int handle_clients(int sd);

/**
 * @brief Handle client request
 *
 * @param sd Valid client socket descriptor
 *
 * @return Error codes from ec enum
 */
int handle_request(int sd);

/**
 * @brief Check validity of file name
 * @details File name for this assignment should not contain some characters
 *
 * @param file File name to check
 *
 * @return true if given file name is valid, false otherwise
 */
bool check_filename(const string &file);

int main(int argc, char *argv[])
{
    int port, sd, ec = E_OK;

    signal(SIGCHLD, catch_child);

    // We need to parse only one parameter, so there's no need to use
    // getopt() or similar library/mechanism
    if(argc == 3 && strcmp(argv[1], "-p") == 0) {
        try {
            char *ptr;
            // Try to convert string to int
            port = strtol(argv[2], &ptr, 10);

            if(*ptr != '\0')
                throw invalid_argument("not a number");

            // Check port range
            if(port < 1 || port > 65535)
                throw range_error("out of range");
        } catch(const exception &e) {
            cerr << "Invalid port (" << e.what() << ")" << endl;
            exit(E_PARAM);
        }
    } else {
        cerr << "Usage: " << argv[0] << " -p <port>" << endl;
        exit(E_PARAM);
    }

    ec = server_setup(port, &sd);
    if(ec != E_OK) {
        cerr << "Server setup failed" << endl;
        exit(ec);
    }

    ec = handle_clients(sd);
    if(ec != E_OK) {
        cerr << "An error has occured during client handling" << endl;
    }

    if(sd != -1)
        close(sd);

    return ec;
}

void catch_child(int sig_num)
{
    int status;
    wait(&status);
}

string ltrim(const string &str)
{
    size_t first = str.find_first_not_of(" \t");
    if(first == string::npos)
        return "";

    return str.substr(first, str.size());
}

int server_setup(int port, int *sd)
{
    int rc, on = 1;
    struct sockaddr_in server_addr;

    *sd = socket(AF_INET, SOCK_STREAM, 0);
    if(*sd < 0) {
        perror("socket() failed");
        exit(E_SOCK);
    }

    rc = setsockopt(*sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
    if(rc < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        return E_SETUP;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    rc = ::bind(*sd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(rc < 0) {
        perror("bind() failed");
        return E_SETUP;
    }

    rc = listen(*sd, CLIENT_QUEUE);
    if(rc < 0) {
        perror("listen() failed");
        return E_SETUP;
    }

    return E_OK;
}

int handle_clients(int sd)
{
    int ec = E_OK, cd;
    socklen_t slen;
    pid_t pid;
    struct sockaddr_in server_addr;

    if(sd == -1) {
        cerr << "Invalid socket" << endl;
        return E_SOCK;
    }

    slen = sizeof(server_addr);

    while(true) {
        cd = accept(sd, (struct sockaddr *)&server_addr, &slen);
        if(cd < 0) {
            perror("accept() failed");
            return E_ACCEPT;
        }

        pid = fork();
        if(pid > 0) {
            // Parent
            close(cd);
        } else if(pid == 0) {
            cout << "[worker] spawned" << endl;
            ec = handle_request(cd);

            close(cd);
            close(sd);

            cout << "[worker] quitting" << endl;
            exit(ec);
        } else {
            perror("fork() failed");
            return E_OTHER;
        }
    }

    return ec;
}

int handle_request(int sd)
{
    int rc;
    float version;
    char buffer[BUFFER_LENGTH] = { 0, };
    string::size_type idx;
    string message, protocol, command, file, sync;
    stringstream is;

    while((rc = read(sd, buffer, BUFFER_LENGTH)) > 0) {
        message.append(buffer, rc);
        if((idx = message.find("\r\n\r\n")) != string::npos) {
            message.erase(idx);
            break;
        }
    }

    if(message.size() == 0) {
        cerr << "[worker] Received empty message" << endl;
        return E_OK;
    }

    is.str(message);

    // Parse protocol, version and command
    is >> protocol >> version >> command;
    // Assign rest of the stream to file
    file.assign(std::istreambuf_iterator<char>(is), {});

    // Edit parsed elements appropriately 
    transform(protocol.begin(), protocol.end(), protocol.begin(), ::toupper);
    transform(command.begin(), command.end(), command.begin(), ::toupper);
    file = ltrim(file);

    // Check protocol
    if(protocol != PROTO_NAME) {
        cerr << "[worker] Received invalid protocol name" << endl;
        ProtoErr err(ProtoErr::PE_INVALID_PROTO);
        return err.send(sd);
    }

    // Version check is not important right now

    // Check command
    if(command != "PUT" && command != "GET") {
        cerr << "[worker] Received invalid command" << endl;
        ProtoErr err(ProtoErr::PE_INVALID_CMD);
        return err.send(sd);
    }

    if(file.size() == 0 || !check_filename(file)) {
        cerr << "[worker] Received invalid file name" << endl;
        ProtoErr err(ProtoErr::PE_INVALID_FILE);
        return err.send(sd);
    }

    memset(buffer, 0, BUFFER_LENGTH);

    if(command == "PUT") {
        cout << "[worker] PUT request for file '" << file << "'" << endl;
        ofstream out;
        try {
            out.open(file, ios::out|ios::binary);
            if(!out.is_open()) {
                perror("open() failed");
                throw runtime_error("unable to open file '" + file + "'"); 
            }
        } catch(const exception &e) {
            cerr << "[worker] " << e.what() << endl;
            ProtoErr err(ProtoErr::PE_PUT_ERROR);
            return err.send(sd);
        }

        ProtoErr status(ProtoErr::PE_OK);
        status.send(sd);

        cout << "[worker] Waiting for client" << endl;
        while((rc = read(sd, buffer, BUFFER_LENGTH)) > 0) {
            sync.append(buffer, rc);
            if((idx = sync.find("\r\n\r\n")) != string::npos) {
                sync.erase(idx);
                break;
            }
        }

        if(sync != "READY")
            return E_OTHER;

        while((rc = read(sd, buffer, BUFFER_LENGTH)) > 0) {
            out.write(buffer, rc);
        }

        cout << "[worker] file saved" << endl;
        out.close();
    } else if(command == "GET") {
        cout << "[worker] GET request for file '" << file << "'" << endl;
        ifstream in;
        try {
            in.open(file, ios::in|ios::binary);
            if(!in.is_open()) {
                perror("open() failed");
                throw runtime_error("unable to open file '" + file + "'"); 
            }
        } catch(const exception &e) {
            cerr << "[worker] " << e.what() << endl;
            ProtoErr err(ProtoErr::PE_GET_ERROR);
            return err.send(sd);
        }

        ProtoErr status(ProtoErr::PE_OK);
        status.send(sd);

        cout << "[worker] Waiting for client" << endl;
        while((rc = read(sd, buffer, BUFFER_LENGTH)) > 0) {
            sync.append(buffer, rc);
            if((idx = sync.find("\r\n\r\n")) != string::npos) {
                sync.erase(idx);
                break;
            }
        }

        if(sync != "READY")
            return E_OTHER;

        while(in.good()) {
            in.read(buffer, BUFFER_LENGTH);
            rc = write(sd, buffer, in.gcount());
            if(rc < 0) {
                perror("[worker] write() failed");
                return E_WRITE;
            }
        }

        cout << "[worker] file sent" << endl;
        in.close();
    }

    return E_OK;
}

bool check_filename(const string &file)
{
    // Slash and backslash test
    if(file.find_first_of("/\\") != string::npos)
        return false;

    // Current dir (.) and parent dir (..) check
    if(file == ".." || file == ".")
        return false;

    return true;
}
