/*
 * File: bit-array.h
 * Author: Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz> BUT FIT
 * Complied with: gcc 4.8.4
 * 
 * This file is part of IJC project and contains macros/functions
 * for manipulation with bit arrays.
 */
#ifndef __BIT_ARRAY_H_DEFINED
#define __BIT_ARRAY_H_DEFINED

#include <stdbool.h>
#include <string.h>

#include "error.h"


typedef unsigned long BitArray_t;

#define BARR_SIZE (sizeof(BitArray_t) * 8)

/**
 * @brief Gets bit on given index from an array.
 * @details Don't call this macro directly, it doesn't check array boundaries. Call BA_get_bit() instead.
 * 
 * @param array Valid pointer to a bit array.
 * @param index Index of requested bit.
 */
#define DU1_GET_BIT_(array, index) ((bool)(array[(index / BARR_SIZE) + 1] & (1UL << (index % BARR_SIZE))))

/**
 * @brief Sets bit on given index in an array to given value.
 * @details Don't call this macro directly, it doesn't check array boundaries. Call BA_set_bit() instead.
 * 
 * @param array Valid pointer to a bit array.
 * @param index Index of bit to set.
 * @param expr Value to set (can be an expression).
 */
#define DU1_SET_BIT_(array, index, expr) \
    (array[(index / BARR_SIZE) + 1] ^= (-((bool)expr) ^ array[(index / BARR_SIZE) + 1]) & (1UL << (index % BARR_SIZE)))

/**
 * @brief Creates statically allocated array. Size of thais array must be known during compilation time.
 * @details This macro should be able to create static or local array.
 * 
 * @param array Array name.
 * @param size Size of an array.
 */
#define BA_create(array, size) \
    BitArray_t array[((size / BARR_SIZE) + ((size % BARR_SIZE > 0) ? 1 : 0)) + 1] = {size, 0, }; 

#ifdef USE_INLINE
    /**
     * @brief Return size of bit array in bits.
     * @details Inline equivalent of BA_size macro
     * 
     * @param array Valid pointer to a bit array.
     * @return Size of bit array in bits.
     */
    inline unsigned long BA_size(BitArray_t *array) {
        return array[0];
    }

    /**
     * @brief Gets bit from given array on given index.
     * @details This function is a wrapper which checks boundaries of given array and calls DU1_GET_BIT_() macro. 
     * 
     * @param array Valid pointer to a bit array.
     * @param index Index of requested bit.
     * 
     * @return Value of a bit on given index from given array.
     */
    inline bool BA_get_bit(BitArray_t *array, unsigned long index) {
        if(index >= array[0]) {
            FatalError("Index is out of range (%lu)\n", index);
        }

        return DU1_GET_BIT_(array, index);
    }

    /**
     * @brief Sets bit on given index in an array to given value.
     * @details This function is a wrapper which checks boundaries of given array and calls DU1_SET_BIT_() macro.
     * 
     * @param array Valid pointer to bit array.
     * @param index Index of bit to set.
     * @param expr Value to set (can be an expression).
     */
    inline void BA_set_bit(BitArray_t *array, unsigned long index, bool expr) {
        if(index >= array[0]) {
            FatalError("Index is out of range (%lu)\n", index);
        }

        DU1_SET_BIT_(array, index, expr);
    }
#else
    /**
     * @brief Return size of bit array in bits.
     * @details Macro equivalent of BA_size inline function
     * 
     * @param array Valid pointer to a bit array.
     * @return Size of bit array in bits.
     */
    #define BA_size(array) array[0]

    /**
     * @brief Gets bit from given array on given index.
     * @details This macro is a wrapper which checks boundaries of given array and calls DU1_GET_BIT_() macro. 
     * 
     * @param array Valid pointer to a bit array.
     * @param index Index of requested bit.
     * 
     * @return Value of a bit on given index from given array.
     */
    #define BA_get_bit(array, index) \
        ((index >= array[0] ? (FatalError("Index %ld mimo rozsah 0..%ld\n", (long)index, (long)array[0]), 0) : DU1_GET_BIT_(array, index)))

    /**
     * @brief Sets bit on given index in an array to given value.
     * @details This function is a wrapper which checks boundaries of given array and calls DU1_SET_BIT_() macro.
     * 
     * @param array Valid pointer to bit array.
     * @param index Index of bit to set.
     * @param expr Value to set (can be an expression).
     */
    #define BA_set_bit(array, index, expr) \
        ((index >= array[0] ? (FatalError("Index %ld mimo rozsah 0..%ld\n", (long)index, (long)array[0]), 0) : DU1_SET_BIT_(array, index, expr)))
#endif

#endif