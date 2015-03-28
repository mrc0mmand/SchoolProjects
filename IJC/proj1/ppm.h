/*
 * File: ppm.h
 * Author: Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz> BUT FIT
 * Complied with: gcc 4.8.4
 * 
 * This file is part of IJC project.
 */
#ifndef __PPM_H_DEFINED
#define __PPM_H_DEFINED

/**
 * @brief Structure for ppm image data.
 * 
 */
struct ppm {
    unsigned xsize; /**< Width */
    unsigned ysize; /**< Height */
    char data[];    /**< Image binary data */
};

/**
 * @brief Loads ppm file into ppm struct.
 * @details Function allocates ppm structure and loads ppm file into it.
 * 
 * @param filename Filename of ppm file.
 * @return Pointer to allocated ppm struct on success, NULL otherwise.
 */
struct ppm *ppm_read(const char *filename);


/**
 * @brief Writes content of ppm structure into file.
 * @details Function saves content of ppm structure into file with format-specific metadata.
 * 
 * @param ppm Pointer to ppm structure.
 * @param filename Destination filename.
 * 
 * @return 0 on success, -1 otherwise.
 */
int ppm_write(struct ppm *p, const char *filename);

#endif