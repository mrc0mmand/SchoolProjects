/*
 * Author: Frantisek Sumsal <xsumsal01@stud.fit.vutbr.cz> BUT FIT
 * Date: 4.4.2015
 * File: htable-htab_clear.c
 * Compiled with: gcc 4.9.2, gcc 4.8.4 and gcc 4.9.1
 * 
 * This file is part of IJC project.
 */

#include <stdlib.h>
#include "htable.h"

void htab_clear(htab_t *t)
{
    if(t == NULL)
        return;

    htab_listitem *item = NULL;
    htab_listitem *item_destroy = NULL;

    for(unsigned int i = 0; i < t->htab_size; i++) {
        item = t->ptr[i];

        while(item != NULL) {
            free(item->key);
            item->data = 0;
            item_destroy = item;
            item = item->next;
            free(item_destroy);
        }
    }
}