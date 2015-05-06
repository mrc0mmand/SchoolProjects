/*
 * Author: Frantisek Sumsal <xsumsal01@stud.fit.vutbr.cz> BUT FIT
 * Date: 4.4.2015
 * File: htable-htab_lookup.c
 * Compiled with: gcc 4.9.2, gcc 4.8.4 and gcc 4.9.1
 * 
 * This file is part of IJC project.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "htable.h"

htab_listitem *htab_lookup(htab_t *t, const char *key)
{
    if(t == NULL || key == NULL)
        return NULL;

    unsigned int htab_index = hash_function(key, t->htab_size);
    htab_listitem *item = t->ptr[htab_index];
    htab_listitem *prev_item = item;

    while(item != NULL) {
        if(strcmp(item->key, key) == 0)
            return item;

        prev_item = item;
        item = item->next;
    }    

    if((item = (htab_listitem *)malloc(sizeof(htab_listitem))) == NULL)
        return NULL;

    if((item->key = (char *)malloc(sizeof(char) * (strlen(key) + 1))) == NULL) {
        free(item);
        return NULL;
    }

    strcpy(item->key, key);
    item->data = 0;
    item->next = NULL;

    if(prev_item == NULL) {
        t->ptr[htab_index] = item;
    } else {
        prev_item->next = item;
    }

    return item;
}