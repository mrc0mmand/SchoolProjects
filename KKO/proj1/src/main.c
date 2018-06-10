/**
 * @author Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz>
 * @brief GIF to BMP conversion application
 * @date 4.4.2018
 * @file main.c
 */
#include <iostream>
#include <unistd.h>

#include "gif2bmp.h"

int main(int argc, char *argv[])
{
    FILE *logfile = NULL;
    FILE *infile = stdin;
    FILE *outfile = stdout;
    size_t ec = 0;
    int c;
    tGIF2BMP gif2bmp_data;

    while((c = getopt(argc, argv, "hi:l:o:")) != -1) {
        switch(c) {
        case 'i':
            infile = fopen(optarg, "rb");
            if(infile == NULL) {
                perror("fopen()");
                exit(EXIT_FAILURE);
            }
            break;
        case 'l':
            logfile = fopen(optarg, "w");
            if(infile == NULL) {
                perror("fopen()");
                exit(EXIT_FAILURE);
            }
            break;
        case 'o':
            outfile = fopen(optarg, "wb");
            if(outfile == NULL) {
                perror("fopen()");
                exit(EXIT_FAILURE);
            }
            break;
        case 'h':
            printf("Usage: %s [-i infile] [-l logfile] [-o outfile]\n",
                    argv[0]);
            exit(EXIT_SUCCESS);
            break;
        }
    }

    ec = gif2bmp(&gif2bmp_data, infile, outfile);

    if(ec == 0 && logfile != NULL) {
        fprintf(logfile, "login = xsumsa01\nuncodedSize = %ld\n"
                "codedSize = %ld\n", gif2bmp_data.gifSize,
                gif2bmp_data.bmpSize);
    }

    fclose(infile);
    fclose(outfile);

    return ec;
}
