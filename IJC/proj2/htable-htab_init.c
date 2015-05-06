/*
 * Author: Frantisek Sumsal <xsumsal01@stud.fit.vutbr.cz> BUT FIT
 * Date: 4.4.2015
 * File: htable-htab_init.c
 * Compiled with: gcc 4.9.2, gcc 4.8.4 and gcc 4.9.1
 * 
 * This file is part of IJC project.
 */

#include <stdlib.h>
#include "htable.h"

htab_t *htab_init(int size)
{
    htab_t *t = NULL;

    if(size < 1) {
        return NULL;
    }

    if((t = (htab_t*)malloc(sizeof(htab_t) + (size * sizeof(htab_listitem)))) == NULL) {
        return NULL;
    }

    for(unsigned int i = 0; i < size; i++) {
        t->ptr[i] = NULL;
    }

    t->htab_size = size;

    return t;
}