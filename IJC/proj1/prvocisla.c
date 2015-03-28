/*
 * File: prvocisla.c
 * Author: Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz> BUT FIT
 * Complied with: gcc 4.8.4
 * 
 * This file is part of IJC project.
 * Program calls function Eratosthenes from eratosthenes.c with
 * N-size bit array and prints last NPRIMES primes.
 */
#include <stdio.h>
#include <stdlib.h>

#include "eratosthenes.h"

#define N 201000000
#define NPRIMES 10

int main(void)
{
    BA_create(a, N);

    Eratosthenes(a);

    unsigned long num[NPRIMES];
    short i_num = 0;

    for(unsigned long i = N - 1; i >= 2 && i_num < NPRIMES; i--) {
        if(BA_get_bit(a, i) == 0) 
            num[i_num++] = i;
    }

    for(short i = i_num - 1; i >= 0; i--)
        printf("%lu\n", num[i]);


    return 0;
}