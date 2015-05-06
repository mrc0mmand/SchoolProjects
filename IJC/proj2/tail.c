/*
 * Author: Frantisek Sumsal <xsumsal01@stud.fit.vutbr.cz> BUT FIT
 * Date: 30.3.2015
 * File: tail.c
 * Compiled with: gcc 4.9.2, gcc 4.8.4 and 4.9.1
 * 
 * This file is part of IJC project.
 * Also, this solution kinda sucks, because it has to read
 * target file twice, which really slows it down.
 * 
 * Benchmarks:
 * # Built-in tail
 * $ tail 30mbfile
 * real    0m0.003s
 * user    0m0.000s
 * sys     0m0.002s
 * 
 * # This tail
 * $./tail 30mbfile
 * real    0m0.101s
 * user    0m0.075s
 * sys     0m0.022s
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Buffer max size */
#define BUFFER_SIZE 1024

/**
 * @brief Checks if given string is a valid integer
 * @details I should've definitely used here strtol(), but
 *          well, this should be enough...
 * 
 * @param str String which contains number to check
 * @return true if given string is a valid integer, false otherwise
 */
bool is_int(const char *str);

/**
 * @brief Converts given string to positive (or zero) integer
 * @details Again, I should've used strol() here, but...
 * 
 * @param str String which should be converted
 * @return Positive (or zero) integer on success, -1 otherwise
 */
int str_to_pint(const char *str);

/**
 * @brief Counts lines in given file descriptor
 * @details Function just counts \n characters in given fd
 * 
 * @param fd Valid pointer to file descriptor
 * @return Line count on success, and... well, this function will surely never fail
 */
int count_lines(FILE *fd);

/**
 * @brief Prints last max lines from file or start+ lines if start is initialized
 * @details Does (or should do) the same as tail command on *NIX systems
 * 
 * @param file Pointer to string with file name. If set to NULL, stdin will be used
 * @param start First line which should be printed. If set to -1, only last max lines will be printed
 * @param max Limits how many lines will be printed. If set to -1, all lines until the end of file will be printed
 */
void parse_file(const char *file, int start, int max);

int main(int argc, char *argv[])
{
    char *file = NULL;
    int start_line = -1;
    int max_line = 10;

    /* This is a little bit messy, but I just didn't want to use getops for one parameter */
    if(argc == 2) { 
        file = argv[1];
    } else if(argc >= 3 && strcmp(argv[1], "-n") == 0) {
        if(strlen(argv[2]) >= 2 && argv[2][0] == '+' && is_int(&argv[2][1])) {
            start_line = str_to_pint(&argv[2][1]);

            if(start_line < 0) {
                fprintf(stderr, "Invalid value for -n parameter.\n");
                exit(1);
            }

            if(start_line > 0)
                start_line--;

            max_line = -1;
        } else if(is_int(argv[2])) {
            max_line = str_to_pint(argv[2]);

            if(max_line < 0) {
                fprintf(stderr, "Invalid value for -n parameter.\n");
                exit(1);
            }
        } else {
            fprintf(stderr, "Invalid value for -n parameter.\n");
            exit(1);
        }

        if(argc >= 4) {
            file = argv[3];
        }
    }

    parse_file(file, start_line, max_line);

    return 0;
}

bool is_int(const char *str)
{
    int x;

    if(sscanf(str, "%d", &x) == 1)
        return true;

    return false;
}

int str_to_pint(const char *str)
{
    int x;

    if(sscanf(str, "%d", &x) == 1 && x >= 0)
        return x;

    return -1;
}

int count_lines(FILE *fd)
{
    int line_count = 0;
    char buffer[BUFFER_SIZE];

    while(fgets(buffer, BUFFER_SIZE, fd) != NULL) {
        if(buffer[strlen(buffer) - 1] == '\n' || feof(fd)) {
            line_count++;
        }
    }

    rewind(fd);

    return line_count;
}

void parse_file(const char *file, int start, int max)
{
    FILE *in = NULL;
    char buffer[BUFFER_SIZE] = {'\0', };
    int current_line = 0;
    int printed_lines = 0;
    int total_lines = 0;
    bool line_end = false;

    if(file != NULL) {
        if((in = fopen(file, "r")) == NULL) {
            fprintf(stderr, "Can't open file %s for reading.\n", file);
            exit(2);
        }
    } else {
        in = stdin;
    }

    total_lines = count_lines(in);
    if(start == -1)
        start = total_lines - max;

    while(fgets(buffer, BUFFER_SIZE, in) != NULL) {
        /* If we got line end in the last iteration, increment line counter */
        if(line_end != false) {
            current_line++;
            line_end = false;
        }

        /* If the (strlen-1)th character is \n, we finally entire line */
        if(buffer[strlen(buffer) - 1] == '\n') {
            line_end = true;
        }

        /* Check if given line is in range for printing */
        if(current_line >= start && (max == -1 || printed_lines < max)) {
            printf("%s", buffer);
            /* If we printed an entire line, increment counter */
            if(line_end != false)
                printed_lines++;
        }

        /* If we printed enough lines, our work here is done */
        if(printed_lines > max && max != -1)
            break;
    }

    if(file != NULL)
        fclose(in);
}