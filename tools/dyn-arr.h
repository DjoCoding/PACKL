#ifndef DYN_ARR_H
#define DYN_ARR_H

#include <stdio.h>
#include <stdlib.h>
#include "error.h"

/*
    Each struct should be defined in that way
    
    struct Dynamic_Array_Structure {
        Dynamic_Array_Type *items;
        size_t count;
        size_t size;
    }

*/

#define DA_INIT(arr, item_size) \
    do { \
        (arr)->size = 1; \
        (arr)->count = 0; \
        (arr)->items = malloc(item_size); \
        if (!((arr)->items)) THROW_ERROR("Could not allocate memory"); \
    } while(0)

#define DA_RESIZE(arr) \
    do { \
        (arr)->size *= 2; \
        (arr)->items = realloc((arr)->items, (arr)->size * sizeof((arr)->items[0])); \
        if (!((arr)->items)) THROW_ERROR("Could not allocate memory"); \
    } while(0)

#define DA_FULL(arr) ((arr)->count >= (arr)->size)

#define DA_APPEND(arr, item) \
    do { \
        if (DA_FULL(arr)) DA_RESIZE(arr); \
        (arr)->items[(arr)->count++] = item; \
    } while(0)

#define DA_REMOVE(arr) free((arr)->items)

#endif // DYN_ARR_H
