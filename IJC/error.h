/*
 * File: error.h
 * Author: Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz> BUT FIT
 * Complied with: gcc 4.8.4
 * 
 * This file is part of IJC project.
 */
#ifndef __ERROR_H_DEFINED
#define __ERROR_H_DEFINED

/**
 * @brief Error codes.
 */
enum ecodes {
    E_OK = 0,   /**< Everything's ok */
    E_IFILE,    /**< Invalid file */
    E_IVAL,     /**< Invalid value */
    E_IRANGE,   /**< Invalid range */
    E_IREAD,    /**< Invalid read */
    E_OOM,      /**< Out of memory */
    E_OTHER     /**< Other error */
};

/**
 * @brief Prints message to standard error output.
 * @details Function behaves exactly like fprintf(stderr, format, ...);
 * 
 * @param fmt Message format (see printf())
 */
void Warning(const char *fmt, ...);

/**
 * @brief Prints message to standard error output and exits program.
 * @details Function behaves exactly like fprintf(stderr, format, ...); exit(1);
 * 
 * @param fmt Message format (see printf())
 */
void FatalError(const char *fmt, ...);

#endif