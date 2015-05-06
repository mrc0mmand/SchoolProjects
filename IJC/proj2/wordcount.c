/*
 * Author: Frantisek Sumsal <xsumsal01@stud.fit.vutbr.cz> BUT FIT
 * Date: 4.4.2015
 * File: wordcount.c
 * Compiled with: gcc 4.9.2, gcc 4.8.4 and gcc 4.9.1
 * 
 * This file is part of IJC project.
 */

#include <stdio.h>
#include <stdlib.h>

#include "htable.h"
#include "io.h"

/* Using prime as hash table size reduces clustering. */
#define HTABLE_SIZE 769

void print_item(const char *key, unsigned int data);

int main(int argc, char const *argv[])
{
    char buffer[128] = {0, };
    htab_t *words = NULL;
    htab_listitem *item = NULL;

    if((words = htab_init(HTABLE_SIZE)) == NULL) {
        fprintf(stderr, "Unable to initialize hash table (out of memory?)\n");
        exit(2);
    }

    while(fgetw(buffer, 128, stdin) != EOF) {
        item = htab_lookup(words, buffer);

        if(item == NULL) {
            fprintf(stderr, "Unable to initialize hash table item (out of memory?)\n");
            exit(2);
        }

        item->data++;
    }

    htab_foreach(words, print_item);

    htab_free(words);

    return 0;
}

void print_item(const char *key, unsigned int data)
{
    printf("%s\t%u\n", key, data);
}