/*
 * File: steg-decode.c
 * Author: Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz> BUT FIT
 * Complied with: gcc 4.8.4
 * 
 * This file is part of IJC project.
 * Program loads a ppm image from file and
 * tries to find a hidden message in it.
 * Message is defined as LSB in bytes of
 * image data on every prime index.
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "ppm.h"
#include "bit-array.h"
#include "error.h"
#include "eratosthenes.h"

#define MAX_BA_SIZE 100000000
#define MSG_SIZE (sizeof(char) * 8)

int main(int argc, char const *argv[])
{
    if(argc == 1) {
        printf("Usage: %s filename\n", argv[0]);
        exit(1);
    }

    struct ppm *image = NULL;
    unsigned long byte_count = 0;
    unsigned long prime_count = 0;
    unsigned long written_bits = 0;

    if((image = ppm_read(argv[1])) == NULL) {
        FatalError("Can't continue without valid file.\n");
    }

    byte_count = 3 * image->xsize * image->ysize;

    /* We need to have some limit in order to be able statically allocate our message array */
    if(byte_count > MAX_BA_SIZE) {
        FatalError("Image is too large to process (max: %ld bytes | got: %ld bytes)\n", MAX_BA_SIZE, byte_count);
    }

    /* Create bit array and find all primes in it */
    BA_create(ba, MAX_BA_SIZE);
    Eratosthenes(ba);

    /* Count primes in our bit array. This value will be used in next statement to allocate our message array */
    for(unsigned long i = 2; i < byte_count; i++) {
        if(BA_get_bit(ba, i) == 0)
            prime_count++;
    }

    /* Initialize an array for our message and set all its bytes to zero */
    char decoded_msg[prime_count / MSG_SIZE];
    memset(decoded_msg, 0, sizeof(decoded_msg));

    for(unsigned long i = 2; i < byte_count; i++) {
        if(BA_get_bit(ba, i) == 0) {
            /* Process prime index */

            if(((written_bits + 1) % MSG_SIZE) == 0) {
                /* Got a full byte, let's check it */

                if(decoded_msg[written_bits / MSG_SIZE] == '\0') {
                    /* Got a nullbyte, that means end of the message */
                    break;
                }

                if(!isprint(decoded_msg[written_bits / MSG_SIZE])) {
                    /* FAIL: Unprintable character */
                    FatalError("Got unprintable character during decoding picture message, can't continue...\n");
                }
            }

            /* Extract first bit from image byte on a prime index and save it into our message array */
            decoded_msg[(written_bits / MSG_SIZE)] ^= (-(image->data[i] & 1) ^ decoded_msg[(written_bits / MSG_SIZE)]) & (1 << (written_bits % MSG_SIZE));
            written_bits++;
        }
    }

    if(decoded_msg[written_bits / MSG_SIZE] != '\0') {
        /* Whoah, unterminated message, get the hell out of there */
        FatalError("Picture message is not terminated with nullbyte, can't continue...\n");
    }

    printf("%s\n", decoded_msg);

    free(image);
    return 0;
}