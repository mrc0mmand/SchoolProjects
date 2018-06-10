/**
 * @author Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz>
 * @file shell.c
 * @date 19.4.2017
 */
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "shell.h"

// Mutexes and condition variables for thread synchronization
pthread_cond_t read_cond;
pthread_cond_t exec_cond;
pthread_mutex_t read_mtx;
pthread_mutex_t exec_mtx;

int main(int argc, char *argv[])
{
    int rc;
    char input_buffer[INBUF_MAX];
    pthread_t pinput;
    pthread_t pexec;

    rc = pthread_create(&pinput, NULL, thread_process_input, (void*)input_buffer);
    if(rc != 0)
        handle_error(rc, "pthread_create()");

    rc = pthread_create(&pexec, NULL, thread_exec, (void*)input_buffer) != 0;
    if(rc != 0)
        handle_error(rc, "pthread_create()");

    rc = pthread_join(pinput, NULL);
    if(rc != 0)
        handle_error(rc, "pthread_join()");

    rc = pthread_cancel(pexec);
    if(rc != 0)
        handle_error(rc, "pthread_cancel()");

    rc = pthread_join(pexec, NULL);
    if(rc != 0)
        handle_error(rc, "pthread_join()");

    return 0;
}

void arg_append(char *arg, cmd_env_t *env)
{
    assert(env != NULL);

    if(env->args_cnt + 1 > env->args_size) {
        env->args = (char**)realloc(env->args, sizeof(*env->args) * (env->args_size + ARGS_CHUNK));
        if(env->args == NULL) {
            perror("realloc()");
            exit(EXIT_FAILURE);
        }

        memset(&env->args[env->args_size], 0, sizeof(*env->args) * ARGS_CHUNK);
        env->args_size += ARGS_CHUNK;
    }

    env->args[env->args_cnt++] = arg;
}

void env_init(cmd_env_t *env)
{
    assert(env != NULL);

    env->args = NULL;
    env->args_cnt = 0;
    env->args_size = 0;
    env->bg = 0;
    env->cmd = NULL;
    env->input = -1;
    env->output = -1;
}

void env_destroy(cmd_env_t *env)
{
    assert(env != NULL);

    free(env->args);
    env->args = NULL;
    env->args_cnt = 0;
    env->args_size = 0;

    if(env->input != -1)
        close(env->input);
    if(env->output != -1)
        close(env->output);
}

int parse_io(char *buffer, char ioc, int *fd, char** tokctx)
{
    int flags;
    char *ptr;
    char *fname;

    ptr = strchr(buffer, ioc);
    if(ptr != NULL) {
        if(*fd != -1) {
            fprintf(stderr, "Error: invalid command\n");
            return 2;
        }

        // Check if the file name immediately follows the I/O redirection
        // character. If not, read next token
        if(*(ptr + 1) != '\0') {
            fname = &ptr[1];
        } else {
            fname = strtok_r(NULL, " ", tokctx);
        }

        if(fname == NULL) {
            fprintf(stderr, "Error: invalid command\n");
            return 2;
        }

        // Set correct flags for each I/O operation
        if(ioc == IN_REDIR) {
            flags = O_RDONLY;
        } else if(ioc == OUT_REDIR) {
            flags = O_WRONLY|O_CREAT|O_TRUNC;
        } else {
            fprintf(stderr, "Error: invalid I/O character\n");
            return 2;
        }

        *fd = open(fname, flags, S_IRUSR|S_IWUSR);
        if(*fd == -1) {
            perror("open()");
            return 2;
        }

        return 0;
    }

    return 1;
}

void sigchld_handler(int sig, siginfo_t *info, void *ucontext)
{
    if(sig != SIGCHLD)
        return;

    printf("Backgroud process %d exited with code %d\n", info->si_pid,
            info->si_status);
}

static void *thread_process_input(void *arg)
{
    int rc;
    char c;
    char *input_buffer = (char*)arg;
    ssize_t rcount;

    rc = pthread_mutex_lock(&exec_mtx);
    if(rc != 0)
        handle_error(rc, "pthread_mutex_lock()");

    while(1) {
        write(STDOUT_FILENO, "$ ", 2);

        rcount = read(STDIN_FILENO, input_buffer, INBUF_MAX);
        if(rcount == INBUF_MAX) {
            if(input_buffer[INBUF_MAX - 1] != '\n') {
                while(read(STDIN_FILENO, &c, 1) == 1) {
                    if(c == '\n')
                        break;
                }
            }

            fprintf(stderr, "Error: input is too long (max %d chars)\n",
                    INBUF_MAX - 1);
            continue;
        }

        // Handle CTRL-D
        if(rcount == 0 || input_buffer[0] == 0x00)
            pthread_exit(EXIT_SUCCESS);

        if(input_buffer[rcount - 1] == '\n')
            // rcount - 1 = newline
            input_buffer[rcount - 1] = '\0';
        else
            // Got control sequence - no newline
            input_buffer[rcount] = '\0';

        // Handle exit
        if(strcmp(input_buffer, "exit") == 0)
            pthread_exit(EXIT_SUCCESS);

        pthread_cond_signal(&read_cond);
        pthread_cond_wait(&exec_cond, &exec_mtx);
    }
}

static void *thread_exec(void *arg)
{
    int rc;
    char *input_buffer = (char*)arg;
    char *ctx;
    char *ptr;
    char *token;
    pid_t pid;
    cmd_env_t env;
    struct sigaction sig_act_orig;
    struct sigaction sig_act = {
        .sa_sigaction = sigchld_handler,
        .sa_flags = SA_SIGINFO
    };
    struct sigaction sig_act_int = {
        .sa_handler = SIG_IGN
    };

    sigemptyset(&sig_act.sa_mask);
    sigemptyset(&sig_act_int.sa_mask);
    // Save previous SIGCHLD handler
    sigaction(SIGCHLD, NULL, &sig_act_orig);
    // Activate custom SIGCHLD handler
    sigaction(SIGCHLD, &sig_act, NULL);
    // Ignore SIGINT
    sigaction(SIGINT, &sig_act_int, NULL);

    rc = pthread_mutex_lock(&read_mtx);
    if(rc != 0)
        handle_error(rc, "pthread_mutex_lock()");

    while(1) {
        pthread_cond_wait(&read_cond, &read_mtx);
        env_init(&env);

        // Check if each IN_REDIR, OUT_REDIR, PROC_BG character
        // is present only once
        if((strchr(input_buffer, IN_REDIR) != strrchr(input_buffer, IN_REDIR)) ||
           (strchr(input_buffer, OUT_REDIR) != strrchr(input_buffer, OUT_REDIR)) ||
           (strchr(input_buffer, PROC_BG) != strrchr(input_buffer, PROC_BG))) {
            fprintf(stderr, "Error: invalid command\n");
            goto end;
        }

        env.cmd = strtok_r(input_buffer, " ", &ctx);
        // Empty buffer
        if(env.cmd == NULL)
            goto end;

        // First item of argv is always command path
        arg_append(env.cmd, &env);

        while((token = strtok_r(NULL, " ", &ctx)) != NULL) {
            // Character for process 'backgrouding' should be the last one
            if(env.bg == 1) {
                fprintf(stderr, "Error: invalid command\n");
                goto end;
            }

            rc = parse_io(token, OUT_REDIR, &env.output, &ctx);
            switch(rc) {
            case 0:
                continue;
                break;
            case 2:
               goto end;
               break;
            }

            rc = parse_io(token, IN_REDIR, &env.input, &ctx);
            switch(rc) {
            case 0:
                continue;
                break;
            case 2:
               goto end;
               break;
            }

            ptr = strchr(token, PROC_BG);
            if(ptr != NULL) {
                if(*(ptr + 1) != '\0') {
                    fprintf(stderr, "Error: invalid command\n");
                    goto end;
                } else {
                    env.bg = 1;
                }
            } else {
                arg_append(token, &env);
            }
        }

        pid = fork();
        if(pid == 0) {
            // Child
            if(env.input != -1) {
                close(0);
                dup2(env.input, STDIN_FILENO);
            }

            if(env.output != -1) {
                close(1);
                dup2(env.output, STDOUT_FILENO);
            }

            // Restore default handler for SIGINT
            sig_act_int.sa_handler = SIG_DFL;
            sigaction(SIGINT, &sig_act_int, NULL);

            execvp(env.cmd, env.args);
            // execv should not return on success
            perror("execvp()");
            exit(EXIT_FAILURE);
        } else if(pid > 0) {
            // Parent
            if(env.bg != 1) {
                // If the process runs in foreground, temporarily
                // restore original SIGCHLD handler to avoid printing
                // SIGCHLD messages
                sigaction(SIGCHLD, &sig_act_orig, NULL);
                waitpid(pid, &rc, 0);
                sigaction(SIGCHLD, &sig_act, NULL);
            }

            // Cleanup zombie (background) processes
            while(waitpid(-1, &rc, WNOHANG) > 0);
        } else {
            // Error
            perror("fork()");
        }

end:
        env_destroy(&env);
        pthread_cond_signal(&exec_cond);
    }
}
