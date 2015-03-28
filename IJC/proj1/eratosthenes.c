/*
 * File: eratosthenes.c
 * Author: Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz> BUT FIT
 * Complied with: gcc 4.8.4
 * 
 * This file is part of IJC project and
 * contains an implementation of
 * Sieve of Eratosthenes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "eratosthenes.h"

void Eratosthenes(BitArray_t *array)
{
    /* Check every number from 2 to sqrt(array_size) */
    for(unsigned long index = 2; index < sqrt(BA_size(array)); index++) {
        if(BA_get_bit(array, index) == 0) {
            /* If processed number is a prime, mark every multiple of that prime as a non-prime */
            for(unsigned long i = 2; i * index < BA_size(array); i++) {
                BA_set_bit(array, (i * index), 1);
            }
        }
    }
}