/*
 * Author: Frantisek Sumsal <xsumsal01@stud.fit.vutbr.cz> BUT FIT
 * Date: 4.4.2015
 * File: io.h
 * Compiled with: gcc 4.9.2, gcc 4.8.4 and gcc 4.9.1
 * 
 * This file is part of IJC project.
 */

#ifndef __IO_H_DEFINED
#define __IO_H_DEFINED

#include <stdio.h>

/**
 * @brief Read a word of max-1 characters from file f and
 *        saves it to string s.
 * @details Word is a sequence of characters separated by
 *          any character from isspace category.
 *          If the word is longer than max-1 characters, it
 *          will be truncated.
 *          This function has also implementation limit of 
 *          127 characters per word.
 * 
 * @param s Valid pointer to string with sufficient size
 * @param max Maximum characters to save to buffer (with \0)
 * @param f Valid pointer to opened file
 * @return Number of read characters on success, -1 on fail and EOF
 *         when function reaches end of file without reading any character
 */
int fgetw(char *s, int max, FILE *f);

#endif