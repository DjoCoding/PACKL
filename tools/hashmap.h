#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "error.h"

#define HASH_MAP_SIZE 20

typedef struct HashMap HashMap;
typedef struct HashMap_Node HashMap_Node;

struct HashMap_Node {
    char *key;
    void *value;
    HashMap_Node *prev, *next;
};

struct HashMap {
    HashMap_Node *nodes[HASH_MAP_SIZE];
    size_t item_size;
};

void hashmap_init(HashMap *self, size_t item_size);
void hashmap_destroy(HashMap *self);
int hashmap_get(HashMap *self, char *key, void *value);
int hashmap_find(HashMap *self, char *key);
int hashmap_add(HashMap *self, char *key, void *value);
int hashmap_remove(HashMap *self, char *key);
int hashmap_update(HashMap *self, char *key, void *value);
int hashmap_empty(HashMap *self);

#endif