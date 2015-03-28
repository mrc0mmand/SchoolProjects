/*
 * File: eratosthenes.h
 * Author: Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz> BUT FIT
 * Complied with: gcc 4.8.4
 * 
 * This file is part of IJC project.
 */
#ifndef __ERATOSTHENES_H_DEFINED
#define __ERATOSTHENES_H_DEFINED

#include "bit-array.h"

/**
 * @brief Implementation of Sieve of Eratosthenes.
 * @details Function calculates prime numbers using a bit array.
 * 
 * @param array Valid pointer to bit array.
 */
void Eratosthenes(BitArray_t *array);

#endif