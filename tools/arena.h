#ifndef ARENA_H
#define ARENA_H

#include <stdio.h>
#include <stdlib.h>
#include "error.h"

#define REGION_MIN_SIZE 1000
#define ARENA_INIT_DEFAULT() arena_init(0)
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct Region Region;
typedef struct Arena Arena;

struct Region {
	char *items;
	size_t count;
	size_t size;
	Region *next;
};

struct Arena {
	Region *head;
	Region *end;
};

Region *region_init(size_t size);
Arena arena_init(size_t size);
void arena_add_region(Arena *self, size_t size);
void *arena_alloc(Arena *self, size_t item_size);
void arena_reset(Arena *self);
void arena_free(Arena *self);
void arena_show(Arena self);

#endif //  ARENA_H
