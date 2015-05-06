/*
 * Author: Frantisek Sumsal <xsumsal01@stud.fit.vutbr.cz> BUT FIT
 * Date: 4.4.2015
 * File: htable-htab_free.c
 * Compiled with: gcc 4.9.2, gcc 4.8.4 and gcc 4.9.1
 * 
 * This file is part of IJC project.
 */

#include <stdlib.h>
#include "htable.h"

void htab_free(htab_t *t)
{
    htab_clear(t);
    free(t);
}