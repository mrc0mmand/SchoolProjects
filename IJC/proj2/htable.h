/*
 * Author: Frantisek Sumsal <xsumsal01@stud.fit.vutbr.cz> BUT FIT
 * Date: 4.4.2015
 * File: htable.h
 * Compiled with: gcc 4.9.2, gcc 4.8.4 and gcc 4.9.1
 * 
 * This file is part of IJC project.
 */

#ifndef __HTABLE_H_INCLUDED
#define __HTABLE_H_INCLUDED

#include <stdlib.h>

typedef struct htab_t htab_t;
typedef struct htab_listitem htab_listitem;

/**
 * @brief Item of hash table
 */
struct htab_listitem {
    char *key;                  /**< Item's key */
    unsigned int data;          /**< Item's data */
    struct htab_listitem *next; /**< Pointer to next item */
};

/**
 * @brief Hash table
 */
struct htab_t {
    unsigned int htab_size; /**< Size of ptr array */
    htab_listitem *ptr[];   /**< Array of pointers to lists of htab_listitem items */
};

/**
 * @brief Executes given function for each item in a hash table
 * 
 * @param t Valid pointer to hash table
 * @param function Valid pointer to function with two parameters: const char * and unsigned int
 */
inline void htab_foreach(htab_t *t, void (*function)(const char *, unsigned int))
{
    htab_listitem *item = NULL;

    for(unsigned int i = 0; i < t->htab_size; i++) {
        item = t->ptr[i];

        while(item != NULL) {
            function(item->key, item->data);
            item = item->next;
        }
    }
}

/**
 * @brief Functions which calculates index for hash table from given key
 * @details This hashing function was taken from www.cse.yorku.ca/~oz/hash.html
 * 
 * @param str String which should be hashed
 * @param int Size of hash table
 * 
 * @return Hashed value of key modulo size of hash table
 */
extern unsigned int hash_function(const char *str, unsigned int htab_size);

/**
 * @brief Initializes hash table and allocates array for list items
 * 
 * @param size Desired size of array for list items
 * @return Pointer to newly allocated hash table on success, NULL otherwise
 */
extern htab_t *htab_init(int size);

/**
 * @brief Dealoates all resources taken by items from given hash table
 * 
 * @param t Pointer to hash table (can be NULL)
 */
extern void htab_clear(htab_t *t);

/**
 * @brief Deallocates hash table (calls also htab_clear for deallocating all items)
 * 
 * @param t Pointer to hash table (can be NULL)
 */
extern void htab_free(htab_t *t);

/**
 * @brief Function searches given key in the hash table. If the key exists,
 *        pointer to that item is returned. Otherwise, function creates
 *        an item for given key in the hash table and returns pointer to it.
 * 
 * @param t Valid pointer to hash table
 * @param key Valid pointer to key which should be searched
 * 
 * @return Pointer to existing/newly created item on success, NULL otherwise (eg. allocation failure)
 */
extern htab_listitem *htab_lookup(htab_t *t, const char *key);

/**
 * @brief Prints statistics for given hash table
 * @details Prints info about maximum/minumum/average list length 
 * 
 * @param t Pointer to valid hash table
 */
extern void htab_statistics(htab_t *t);

/**
 * @brief Removes given item from the hash table
 * 
 * @param t Valid pointer to hash table
 * @param key Valid pointer to key which should be removed
 */
extern void htab_remove(htab_t *t, const char *key);

#endif