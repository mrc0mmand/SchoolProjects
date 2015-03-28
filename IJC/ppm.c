/*
 * File: ppm.c
 * Author: Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz> BUT FIT
 * Complied with: gcc 4.8.4
 * 
 * This file is part of IJC project and 
 * contains functions for reading and saving
 * ppm files.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ppm.h"
#include "error.h"

struct ppm *ppm_read(const char *filename)
{
    FILE *in = NULL;
    short ec = E_OK;
    /* Metadata:
     * type: ppm file type
     * xsize: width of ppm picture
     * ysize: height of ppm picture
     * colormode: ugh, just color mode
     */
    char type[3] = "\0";
    long int xsize = -1;
    long int ysize = -1;
    int colormode = -1;
    unsigned long byte_counter = 0;
    unsigned long blob_size = 0;
    struct ppm *image = NULL;
    char byte = 0;

    if((in = fopen(filename, "rb")) == NULL) {
        Warning("Couldn't open file %s\n", filename);
        return NULL;
    }

    /* Check & load file metadata - maybe goto would be better for this, but I would probably end with my head on a spike for that */
    if(ec == E_OK) {
        if(fscanf(in, "%2s %ld %ld %d\n", type, &xsize, &ysize, &colormode) != 4) {
            Warning("Couldn't read metadata from file %s\n", filename);
            ec = E_IREAD;
        }

        if(ec == E_OK && strcmp(type, "P6") != 0) {
            Warning("Invalid image type %s (supported types: P6)\n", type);
            ec = E_IVAL;
        }

        if(ec == E_OK && (xsize < 0 || ysize < 0)) {
            Warning("Image size is out of range (x: %ld | y: %ld)\n", xsize, ysize);
            ec = E_IRANGE;
        }

        if(ec == E_OK && colormode != 255) {
            Warning("Invalid color mode %ld (supported modes: 255)\n", colormode);
            ec = E_IVAL;
        }

        if(ec == E_OK) {
            blob_size = 3 * xsize * ysize;

            if((image = malloc(sizeof(struct ppm) + blob_size)) == NULL) {
                Warning("Couldn't allocate memory for image data\n");
                ec = E_OOM;
            } else {
                image->xsize = xsize;
                image->ysize = ysize;

                while(fread(&byte, 1, 1, in) == 1 && byte_counter < blob_size) {
                    image->data[byte_counter++] = byte;
                }

                if(fread(&byte, 1, 1, in) == 1 || byte_counter < blob_size) {
                    Warning("Image data blob size doesn't correspond to image size. (x: %ld | y: %ld | blob size: %ld | read: %ld)\n", 
                            xsize, ysize, blob_size, byte_counter);
                    ec = E_IVAL;
                }
            }
        }
    }

    if(ec != E_OK) {
        free(image); image = NULL;
    }

    fclose(in);
    return (ec == E_OK) ? image : NULL;
}

int ppm_write(struct ppm *p, const char *filename)
{
    if(p == NULL) {
        Warning("Invalid ppm pointer.\n");
        return -1;
    }

    FILE *out = NULL;
    short ec = 0;
    unsigned long byte_count = 3 * p->xsize * p->ysize;

    if((out = fopen(filename, "wb")) == NULL) {
        Warning("Couldn't open file %s\n", filename);
        return -1;
    }

    if(fprintf(out, "P6\n%u %u\n255\n", p->xsize, p->ysize) == 0) {
        Warning("Couldn't write image metadata into file %s\n", filename);
        ec = -1;
    }

    if(ec == E_OK && fwrite(p->data, 1, byte_count, out) != byte_count) {
        Warning("Couldn't write image data into file %s\n", filename);
        ec = -1;
    }

    fclose(out);
    return ec;
}