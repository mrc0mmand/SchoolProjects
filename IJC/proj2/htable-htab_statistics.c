/*
 * Author: Frantisek Sumsal <xsumsal01@stud.fit.vutbr.cz> BUT FIT
 * Date: 4.4.2015
 * File: htable-htab_statistics.c
 * Compiled with: gcc 4.9.2, gcc 4.8.4 and gcc 4.9.1
 * 
 * This file is part of IJC project.
 */

#include <stdio.h>
#include <stdlib.h>
#include "htable.h"

void htab_statistics(htab_t *t)
{
    unsigned int min = 0;
    unsigned int max = 0;
    unsigned int count = 0;
    float avg = 0.0;
    htab_listitem *item = NULL;

    for(unsigned int i = 0; i < t->htab_size; i++) {
        item = t->ptr[i];

        while(item != NULL) {
            count++;
            item = item->next;
        }

        if(count > max)
            max = count;

        if(i == 0)
            min = max;

        if(count < min)
            min = count;

        avg += count;
        count = 0;
    }

    avg /= t->htab_size;

    printf("HTable statistics: \n"
            "Min. list length: %u\n"
            "Max. list length: %u\n"
            "Avg. list length: %.2f\n",
            min, max, avg);
}