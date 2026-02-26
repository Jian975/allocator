#include<stdio.h>
#include<stdint.h>
#ifndef ALLOCATOR_H
#define ALLOCATOR_H
#define METADATA_SIZE 51

typedef struct node {
    int16_t size;
    int8_t allocated;
    void * address;
} node_t;

void initialize_allocator(int heap_size);

void * my_malloc(int size);

void my_free(void * address);

void print_memory();
#endif