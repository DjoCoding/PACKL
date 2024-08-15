#ifndef VECTOR_H
#define VECTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/*
    THE VECTOR STRUCTURE 
    {
        void *header;
        void *items;
    }
*/

#define VECTOR(T) T*
#define VECTOR_PUSH(v, i) vector_push((void **)&(v), sizeof(i), (void *)&(i))
#define VECTOR_RESET(v) vector_reset((void **)&(v))
#define VECTOR_REMOVE(v) vector_remove((void **)&(v))

#define VECTOR_INITIAL_SIZE 100

typedef struct Vector_Header Vector_Header;

struct Vector_Header {
    size_t count;
    size_t size;
};

Vector_Header *vect_header_init() {
    Vector_Header *header = (Vector_Header *)malloc(sizeof(*header));
    assert(header != NULL && "ERROR: `vect_header_init` failed to allocate memory\n");
    header->count = 0;
    header->size = VECTOR_INITIAL_SIZE;
    return header;
}

void **vector_init() {
    void **vector = (void **)malloc(2 * sizeof(void *));
    assert(vector != NULL && "ERROR: `vector_init` failed\n");
    return vector;
}

Vector_Header *vector_get_header(void **vect) {
    assert(*vect != NULL && "ERROR: `vector_get_header` failed, cannot get the header of a NULL ptr\n");
    Vector_Header *header = (Vector_Header *)(*vect - 1);
    return header;
}

void vector_resize(void **vect, size_t item_size) {
    Vector_Header *header = vector_get_header(vect);
    header->size *= 2;
    *vect = realloc(*vect, header->size * item_size);
    assert(*vect != NULL && "ERROR: `vector_resize` failed\n");
}

void vector_push(void **vect, size_t item_size, void *item) {
    void **structure = 0;

    if (!*vect) {
        structure = vector_init();

        // SET THE HEADER
        structure[0] = (void *)vect_header_init();

        // SET THE ARRAY
        structure[1] = malloc(VECTOR_INITIAL_SIZE * item_size);
        assert(structure[1] != NULL && "ERROR: `vector_push` failed\n");

        // UPDATE THE ARRAY POINTER
        *vect = structure[1];
    } else {
        Vector_Header *header = vector_get_header(vect);
        if(header->count >= header->size) { vector_resize(vect, item_size); }
    }

    memcpy(*vect + header->count * item_size, item, item_size);
    header->count++;
}

void vector_reset(void **vect) {
    assert(*vect && "ERROR: `vector_reset` failed\n");
    Vector_Header *header = vector_get_header(vect);
    header->count = 0;
}

void vector_remove(void **vect) {
    assert(*vect && "ERROR: `vector_remove` failed\n");
    Vector_Header *header = vector_get_header(vect);
    void *items = *vect;
    free(header);
    free(items);
}


#endif // VECTOR_H