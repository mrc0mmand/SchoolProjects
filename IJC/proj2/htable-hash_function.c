/*
 * Author: Frantisek Sumsal <xsumsal01@stud.fit.vutbr.cz> BUT FIT
 * Date: 4.4.2015
 * File: htable-hash_function.c
 * Compiled with: gcc 4.9.2, gcc 4.8.4 and gcc 4.9.1
 * 
 * This file is part of IJC project.
 */

#include "htable.h"

unsigned int hash_function(const char *str, unsigned int htab_size)
{
    unsigned int hash = 0;

    for(const unsigned char *pc = (const unsigned char *)str; *pc != '\0'; pc++) {
        hash = 65599 * hash + *pc;
    }

    return hash % htab_size;
}