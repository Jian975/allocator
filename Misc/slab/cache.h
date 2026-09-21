#ifndef CACHE_H
#define CACHE_H
#include "slab.h"
#define CACHE_SIZE 8
//for now let's do slabs of size 2 bytes, 4, bytes, 8 bytes, etc.
//let's have 8 slabs in total, so max is 256 bytes

//list of slabs of increasing sizes
typedef struct cache {
    slab_t * slab;
    struct cache * next;
} cache_t;

int to_index(int size);

cache_t * initiate_cache();

void * cache_malloc(cache_t * cache, int size);

void cache_free(cache_t * cache, void * data);

#endif