/**
 * @author Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz>
 * @file fork.c
 * @date 10.4.2018
 */

#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED 1 /* XPG 4.2 - needed for WCOREDUMP() */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * @brief Print current process information
 *
 * @param label Process label
 */
void print_pinfo(const char *label)
{
    printf("%s identification: \n", label);
    printf("\tpid = %d,\tppid = %d,\tpgrp = %d\n", getpid(), getppid(),
            getpgrp());
    printf("\tuid = %d,\tgid = %d\n", getuid(), getgid());
    printf("\teuid = %d,\tegid = %d\n", geteuid(), getegid());
}

/**
 * @brief Print details about process exit status
 *
 * @param label Process label
 * @param pid Process ID
 * @param wstatus Status code received from wait/waitpid function
 */
void print_pexit(const char *label, pid_t pid, int wstatus)
{
    int coredump = 0;

    printf("%s exit (pid = %d):", label, pid);

    if(WIFEXITED(wstatus)) {
        printf("\tnormal termination (exit code = %d)\n",
                WEXITSTATUS(wstatus));
    } else if(WIFSIGNALED(wstatus)) {
#ifdef WCOREDUMP
        if(WCOREDUMP(wstatus))
            coredump = 1;
#endif
        printf("\tsignal termination %s(signal = %d)\n",
                (coredump) ? "with core dump " : "", WTERMSIG(wstatus));
    } else {
        printf("\tunknown type of termination\n");
    }
}

int main(int argc, char *argv[])
{
    pid_t pid;
    int wstatus;

    if(argc < 2) {
        fprintf(stderr, "Usage: %s binary_path [args]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    print_pinfo("grandparent");

    pid = fork();
    if(pid == 0) {
        // Child (parent)
        print_pinfo("parent");

        pid = fork();
        if(pid == 0) {
            // Child (child)
            print_pinfo("child");
            if(execv(argv[1], &argv[1]) < 0) {
                perror("execv()");
                exit(EXIT_FAILURE);
            }
        } else if(pid > 0) {
            // Parent (parent)
            waitpid(pid, &wstatus, 0);
            print_pexit("child", pid, wstatus);
        } else {
            perror("fork()");
            exit(EXIT_FAILURE);
        }
    } else if(pid > 0) {
        // Parent (grandparent)
        waitpid(pid, &wstatus, 0);
        print_pexit("parent", pid, wstatus);
    } else {
        perror("fork()");
        exit(EXIT_FAILURE);
    }

    return 0;
}

