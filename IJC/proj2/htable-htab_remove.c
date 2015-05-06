/*
 * Author: Frantisek Sumsal <xsumsal01@stud.fit.vutbr.cz> BUT FIT
 * Date: 4.4.2015
 * File: htable-htab_remove.c
 * Compiled with: gcc 4.9.2, gcc 4.8.4 and gcc 4.9.1
 * 
 * This file is part of IJC project.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "htable.h"

void htab_remove(htab_t *t, const char *key)
{
    htab_listitem *item = NULL;
    htab_listitem *prev_item = NULL;

    for(unsigned int i = 0; i < t->htab_size; i++) {
        item = t->ptr[i];
        prev_item = NULL;

        while(item != NULL) {
            if(strcmp(item->key, key) == 0) {
                if(prev_item == NULL) {
                    t->ptr[i] = item->next;
                } else {
                    prev_item->next = item->next;
                }

                free(item->key);
                free(item);

                return;
            }

            prev_item = item;
            item = item->next;
        }
    }
}