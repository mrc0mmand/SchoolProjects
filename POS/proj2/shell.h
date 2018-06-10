/**
 * @author Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz>
 * @file shell.h
 * @date 19.4.2017
 */
#ifndef __SHELL_H_INCLUDED
#define __SHELL_H_INCLUDED

#include <stdio.h>
#include <stdint.h>

#define ARGS_CHUNK 10 /**< Allocation unit for an argument array */
#define INBUF_MAX 513 /**< Maximum line size + 1 */
#define IN_REDIR '<'  /**< Input redirection character */
#define OUT_REDIR '>' /**< Output redirection character */
#define PROC_BG '&'   /**< Process backgrounding character */

/**
 * @brief Error handler for pthread_* functions
 *
 * @param en Error number
 * @param msg Error message
 */
#define handle_error(en, msg) \
    do { \
        errno = en; \
        perror(msg); \
        exit(EXIT_FAILURE); \
    } while(0)

/**
 * @brief Structure for command environment data
 */
typedef struct {
    uint8_t bg;     /**< Run the process in background */
    int args_cnt;   /**< Count of command arguments (== argc) */
    int args_size;  /**< Size of the arguments array */
    int input;      /**< Input file descriptor for given command */
    int output;     /**< Output file descriptor for given command */
    char *cmd;      /**< Command name/path */
    char **args;    /**< Command arguments (== argv) */
} cmd_env_t;

/**
 * @brief Append a new argument to the argument array (cmd_env_t.args)
 *
 * @param arg Argument to append (null terminated string)
 * @param env Valid pointer to the cmd_env_t structure
 */
void arg_append(char *arg, cmd_env_t *env);

/**
 * @brief Initialize the cmd_env_t structure
 *
 * @param env Valid pointer to the cmd_env_t structure
 */
void env_init(cmd_env_t *env);

/**
 * @brief Clean up the cmd_env_t structure
 *
 * @param Valid pointer to the cmd_env_t structure
 */
void env_destroy(cmd_env_t *env);

/**
 * @brief Process input/output redirections from the token string
 * @details I/O redirection characters are defined by IN_REDIR and
 *          OUT_REDIR constants
 *
 * @param buffer String to process
 * @param ioc I/O character to process (IN_REDIR/OUT_REDIR)
 * @param fd Pointer to ia variable where the new FD should be stored in
 * @param tokctx Context storage for strtok_r function
 *
 * @returns 0 if the given redirection character was found and correctly
 *          processed, 1 when no redirection character was found in the
 *          given buffer, 2 on error
 */
int parse_io(char *buffer, char ioc, int *fd, char** tokctx);

/**
 * @brief User input handler (should be executed in a separate thread)
 * @details Function reads user input and forwards it to the command
 *          execution handler to process
 *
 * @param arg Pointer to a shared input buffer
 */
static void *thread_process_input(void *arg);

/**
 * @brief Command execution handler (should be executed in a separate thread)
 * @details Function parses given user input and executes processed command
 *
 * @param arg Pointer to a shared input buffer
 */
static void *thread_exec(void *arg);

#endif
