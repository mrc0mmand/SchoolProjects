/*
 * Author: Frantisek Sumsal <xsumsal01@stud.fit.vutbr.cz> BUT FIT
 * Date: 4.4.2015
 * File: io.c
 * Compiled with: gcc 4.9.2, gcc 4.8.4 and gcc 4.9.1
 * 
 * This file is part of IJC project.
 */

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_WORD_SIZE 127

int fgetw(char *s, int max, FILE *f)
{
    static bool trunc_message = false;
    bool trunc_output = false;
    unsigned int read_chars = 0;
    int c = '\0';

    if(max < 0)
        return -1;

    while((c = fgetc(f)) != EOF) {
        if(read_chars == 0 && isspace(c) != false)
            continue;

        if(isspace(c) != false)
            break;

        if(read_chars == max) {
            trunc_output = true;
        }

        if(read_chars == MAX_WORD_SIZE) {
            if(trunc_message == false) {
                fprintf(stderr, "%s: Current word is too long, truncating all words longer than %d characters\n", __FUNCTION__, MAX_WORD_SIZE);
                trunc_message = true;
            }

            trunc_output = true;
        }

        if(trunc_output == false)
            s[read_chars++] = c;
    }

    s[read_chars] = '\0';

    return (read_chars == 0) ? EOF : read_chars;
}