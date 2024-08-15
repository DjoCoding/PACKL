#ifndef PACKL_ERROR_H
#define PACKL_ERROR_H

#include "../tools/error.h"

#define PACKL_ERROR(filename, ...) \
    do { \
        fprintf(stderr, "%s: ", filename); \
        THROW_ERROR(__VA_ARGS__); \
    } while(0)

#endif 