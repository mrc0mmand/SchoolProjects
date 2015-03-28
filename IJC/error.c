/*
 * File: error.c
 * Author: Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz> BUT FIT
 * Complied with: gcc 4.8.4
 * 
 * This file is part of IJC project.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

void Warning(const char *fmt, ...)
{
    va_list args;
    char buff[strlen(fmt) + 7];

    strcpy(buff, "CHYBA: ");
    strcat(buff, fmt);

    va_start(args, fmt);
    vfprintf(stderr, buff, args);
    va_end(args);
}

void FatalError(const char *fmt, ...)
{
    va_list args;
    char buff[strlen(fmt) + 7];

    strcpy(buff, "CHYBA: ");
    strcat(buff, fmt);

    va_start(args, fmt);
    vfprintf(stderr, buff, args);
    va_end(args);

    exit(1);
}
