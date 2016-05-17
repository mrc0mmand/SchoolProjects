/**
 * @file webclient.c
 * @author Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz
 * @date 27.3.2016
 * @brief Basic socket web client
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <regex.h>

#define MAX_HOSTNAME_LEN 255
#define MAX_REDIRECT 5
#define MAX_ERROR_MSG 255
#define BUFFER_LENGTH 4096
#define HEADER_CHUNK 512
#define USER_AGENT "webclient 1.0"
#define DEFAULT_FILE "index.html"

enum ec {
    E_OK = 0,
    E_SOCKET,
    E_HOST,
    E_CONNECT,
    E_SEND,
    E_ALLOC,
    E_REGEX,
    E_INPUT,
    E_OUTPUT,
    E_REDIR,
    E_HTTP
};

enum state {
    S_NONE = 0,
    S_R,
    S_RN,
    S_RNR,
    S_RNRN,
    S_DONE
};

/**
 * @brief Structure containing server information for easier passing between
 *        functions
 */
struct server_info {
    char *server_name;  /**< Server name without protocol (www.fit.vutbr.cz) */
    char *location;     /**< Target location (/index.html) */
    char *filename;     /**< Destination file name (index.html) */
    int port;           /**< Server port (80) */
};

/**
 * @brief Simple auto-expanding buffer
 */
struct buff {
    size_t size;    /**< Current max size */
    size_t idx;     /**< Current free index */
    char *data;     /**< Data pool */
};

/**
 * @brief Copies at most n bytes from string str and allocates a new memory
 *        chunk for it
 *
 * @param str Source string
 * @param n Max bytes to copy
 *
 * @return Allocated string with desired content on success
 */
char *strndup(const char *str, size_t n);

/**
 * @brief Split URL into members of server_info structure
 *
 * @param path Source URL
 * @param s_info Valid pointer to server_info structure where the split URL
 *               will be saved
 */
void split_path(char *path, struct server_info *s_info);

/**
 * @brief Gets a file from a location specified in server_info structure
 * @details Destination is specified by filename member of server_info
 *          structure
 *
 * @param s_info Valid pointer to server_info structure
 *
 * @return Status code specified by ec enum
 */
int get_file(struct server_info *s_info);

/**
 * @brief Builds a HTTP/1.1 request from given server and location
 *
 * @param server String containing target server
 * @param location String containing target location
 *
 * @return Allocated string with crafted request on success
 */
char *build_request(char *server, char *location);

/**
 * @brief Allocates buffer array in buff structure on the first call, resizes
 *        it by size bytes on each subsequent call
 *
 * @param b Valid pointer to buff structure
 * @param size Size which the buffer will be resized by
 *
 * @return Resized buffer on success. Same pointer is also saved into data
 *         pointer of buff structure
 */
char *buffer_resize(struct buff *b, size_t size);

/**
 * @brief Puts char into buff structure. If the buffer is not big enough, calls
 *        buffer_resize with HEADER_CHUNK as size parameter
 *
 * @param b Valid pointer to buff structure
 * @param c Char to be put into the buffer
 */
void buffer_putc(struct buff *b, char c);

/**
 * @brief Deallocates dynamically allocated memory in buff structure and sets
 *        default values of all its members
 *
 * @param b Pointer to buff structure
 */
void buffer_free(struct buff *b);

/**
 * @brief Deallocates dynamically allocatd memory in server_info structure and
 *        sets default values of all its members
 *
 * @param b Pointer to server_info structure
 */
void free_server_info(struct server_info *s);

/**
 * @brief Takes received header response saved in buffer b and optionally
 *        modifies members of server_info structure (depends on header
 *        response)
 *
 * @param b Valid pointer to buff structure with header response
 * @param s Valid pointer to server_info structure pre-filled with information
 *          about previous request
 *
 * @return E_OK on 2xx, E_REDIR on 3xx and E_HTTP on 4xx and 5xx status codes
 */
int process_header(struct buff *b, struct server_info *s);

int main(int argc, char *argv[])
{
    int ec = E_OK;
    int redir = 0;
    struct server_info s_info = { .server_name = NULL, .location = NULL, 
                                  .port = 0 };

    if(argc <= 1) {
        printf("Usage: %s URL\n", argv[0]);
        exit(0);
    }

    split_path(argv[1], &s_info);
    while((ec = get_file(&s_info)) == E_REDIR) {
        if(++redir > MAX_REDIRECT)
            break;
    }

#ifdef IPK_DEBUG
    printf("Server: %s\nPath: %s\nFile: %s\nPort: %d\n", s_info.server_name,
                                                         s_info.location,
                                                         s_info.filename,
                                                         s_info.port);
#endif
    free_server_info(&s_info);

    return ec;
}

char *strndup(const char *str, size_t n)
{
    if(str == NULL)
        return NULL;

    char *s = malloc(sizeof(char) * n + 1);

    if(s == NULL) {
        perror("malloc() failed");
        exit(E_ALLOC);
    }

    memcpy(s, str, n);
    s[n] = '\0';

    return s;
}

void split_path(char *path, struct server_info *s_info)
{
    enum regex_parts {
        REGX_URL = 5,
        REGX_PORT = 7,
        REGX_LOC = 8
    };
    const int n_match = 10;
    char regex[] = "^(([^:/?#]+):)?(//(([^/?#:]*)(\\:([0-9]+))?)))?([^?#]*)"
                   "(\\?([^#]*))?(#(.*))?";

    regex_t r;
    regmatch_t m[n_match];
    int rc = 0;

    if(path == NULL || s_info == NULL)
        return;

    rc = regcomp(&r, regex, REG_EXTENDED);
    if(rc != 0) {
        char err[MAX_ERROR_MSG];
        regerror(rc, &r, err, MAX_ERROR_MSG);
        fprintf(stderr, "Regex compilation fail: %s\n", err);
        exit(E_REGEX);
    }

    if(regexec(&r, path, n_match, m, 0) == 0) {
        s_info->server_name = strndup(&path[m[REGX_URL].rm_so],
                                      (m[REGX_URL].rm_eo - m[REGX_URL].rm_so));
        if(m[REGX_LOC].rm_eo - m[REGX_LOC].rm_so <= 0) {
            s_info->location = strndup("/", 1);
        } else {
            int spaces = 0;
            for(int i = m[REGX_LOC].rm_so; i < m[REGX_LOC].rm_eo; i++) {
                if(path[i] == ' ')
                    spaces++;
            }

            int size = (m[REGX_LOC].rm_eo - m[REGX_LOC].rm_so) + spaces * 3;
            s_info->location = calloc(size + 1, sizeof(char));
            if(s_info->location == NULL) {
                perror("calloc() failed");
                exit(E_ALLOC);
            }

            for(int i = m[REGX_LOC].rm_so, j = 0; i < m[REGX_LOC].rm_eo; i++, j++) {
                if(path[i] == ' ') {
                    strcat(s_info->location, "%20");
                    j += 2;
                } else {
                    s_info->location[j] = path[i];
                }
            }
        }

        if(m[REGX_PORT].rm_eo - m[REGX_PORT].rm_so <= 0) {
            s_info->port = 80;
        } else {
            size_t s = m[REGX_PORT].rm_eo - m[REGX_PORT].rm_so;
            char buff[s + 1];
            memcpy(buff, &path[m[REGX_PORT].rm_so], s);
            buff[s] = '\0';
            s_info->port = atoi(buff);
        }

        char *fp = &path[m[REGX_LOC].rm_so];
        char *next = NULL;

        fp[m[REGX_LOC].rm_eo] = '\0';
        while((next = strpbrk(fp + 1, "\\/")))
            fp = next;

        if(s_info->location != fp)
            fp++;

        if((next = strchr(fp, '.')) != NULL && *++next != '\0')
            s_info->filename = strndup(fp, strlen(fp));
        else
            s_info->filename = strndup(DEFAULT_FILE, strlen(DEFAULT_FILE));

#ifdef IPK_DEBUG
        for(int idx = 0; idx < n_match; idx++) {
            char b[1024];
            memcpy(b, &path[m[idx].rm_so], m[idx].rm_eo - m[idx].rm_so);
            b[m[idx].rm_eo - m[idx].rm_so] = '\0';
            printf("[#%d] %s\n", idx, b);
        }
#endif
    } else {
        fprintf(stderr, "Invalid URL %s\n", path);
        exit(E_INPUT);
    }

    regfree(&r);
}

int get_file(struct server_info *s_info)
{
    int sd = -1;
    int rc = 0;
    int ec = E_OK;
    int bytes = 0;
    int idx = 0;
    short int state = S_NONE;
    char buffer[BUFFER_LENGTH] = {0,};
    char *request = NULL;
    FILE *of = NULL;
    struct buff header = { .size = 0, .idx = 0, .data = NULL };
    struct sockaddr_in server_addr;
    struct hostent *hostp;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd < 0) {
        perror("socket() failed");
        return E_SOCKET;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(s_info->port);
    server_addr.sin_addr.s_addr = inet_addr(s_info->server_name);

    // Server string was not an IP address
    if(server_addr.sin_addr.s_addr == INADDR_NONE) {
        hostp = gethostbyname(s_info->server_name);
        if(hostp == NULL) {
            fprintf(stderr, "Unknown host: %s\n", s_info->server_name);
            ec = E_HOST;
            goto cleanup;
        }

        memcpy(&server_addr.sin_addr, hostp->h_addr_list[0],
               sizeof(server_addr.sin_addr));
    }

    rc = connect(sd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(rc < 0) {
        perror("connect() failed");
        ec = E_CONNECT;
        goto cleanup;
    }

    request = build_request(s_info->server_name, s_info->location);
    if(request == NULL) {
        fprintf(stderr, "Couldn't create HTTP/1.1 request\n");
        goto cleanup;
    }

    rc = send(sd, request, strlen(request), 0);
    if(rc < 0) {
        perror("send() failed");
        ec = E_SEND;
        goto cleanup;
    }

    buffer_resize(&header, HEADER_CHUNK);

    while((bytes = recv(sd, buffer, BUFFER_LENGTH - 1, 0)) > 0) {
        idx = 0;
        /* Header delimiter (\r\n\r\n) can be split into multiple
         * packets, so we can't use strstr or similar string comparison
         */
        for(int i = 0; i < bytes && state != S_DONE; i++) {
            switch(state) {
            case S_NONE:
                if(buffer[i] == '\r')
                    state = S_R;
                else
                    state = S_NONE;
                break;
            case S_R:
                if(buffer[i] == '\n')
                    state = S_RN;
                else
                    state = S_NONE;
                break;
            case S_RN:
                if(buffer[i] == '\r')
                    state = S_RNR;
                else
                    state = S_NONE;
                break;
            case S_RNR:
                if(buffer[i] == '\n') {
                    state = S_DONE;
                    buffer_putc(&header, '\0');
                    ec = process_header(&header, s_info);
                    if(ec == E_OK) {
                        of = fopen(s_info->filename, "wb");
                        if(of == NULL) {
                            perror("fopen() failed");
                            ec = E_OUTPUT;
                            goto cleanup;
                        }
                    }

                    idx = i + 1;
                    continue;
                } else {
                    state = S_NONE;
                }
                break;
            }

            if(buffer[i] != '\r')
                buffer_putc(&header, buffer[i]);

        }

        if(ec == E_OK && state == S_DONE) {
            fwrite(&buffer[idx], sizeof(char), bytes - idx, of);
        }
    }

cleanup:
    if(sd != -1)
        close(sd);

    buffer_free(&header);
    free(request);
    if(of != NULL)
        fclose(of);

    return ec;
}

char *build_request(char *server, char *location)
{
    char req_templ[] = "GET /%s HTTP/1.1\r\n"
                       "Host: %s\r\n"
                       "User-Agent: %s\r\n"
                       "Connection: close\r\n\r\n";
    char *request = NULL;

    if(location[0] == '/')
        location++;

    request = malloc(sizeof(char) * (strlen(server) + strlen(location) +
                                     strlen(req_templ) + strlen(USER_AGENT)));
    if(request == NULL) {
        perror("malloc failed");
    } else {
        sprintf(request, req_templ, location, server, USER_AGENT);
    }

    return request;
}

char *buffer_resize(struct buff *b, size_t size)
{
    char *n = NULL;

    n = realloc(b->data, b->size + size);

    if(n == NULL) {
        perror("malloc()/realloc() failed");
        exit(E_ALLOC);
    }

    b->data = n;
    b->size += size;
    return n;
}

void buffer_putc(struct buff *b, char c)
{
    if(b->idx >= b->size)
        buffer_resize(b, HEADER_CHUNK);

    b->data[b->idx++] = c;
}

void buffer_free(struct buff *b)
{
    free(b->data);
    b->data = NULL;
    b->size = 0;
    b->idx = 0;
}

void free_server_info(struct server_info *s)
{
    if(s == NULL)
        return;

    free(s->server_name);
    free(s->location);
    free(s->filename);
    s->server_name = NULL;
    s->location = NULL;
    s->filename = NULL;
    s->port = 0;
}

int process_header(struct buff *b, struct server_info *s)
{
    char msg[64];
    int status;

    sscanf(b->data, "HTTP/1.1 %d %63[0-9a-zA-Z ]s", &status, msg);

    if(status >= 200 && status < 300) {
        // 2xx - OK
        return E_OK;
    } else if(status >= 300 && status < 400) {
        // 3xx - redirects
        char *pstr = strstr(b->data, "\nLocation: ");

        if(pstr) {
            char buff[b->size];
            sscanf(pstr, "\nLocation: %s", buff);
            free_server_info(s);
            split_path(buff, s);

            return E_REDIR;
        }
    } else if(status >= 400 && status < 600) {
        // 4xx and 5xx - client and server errors
        fprintf(stderr, "Error: %d - %s\n", status, msg);
    }

    return E_HTTP;
}
