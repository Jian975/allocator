#include<stdio.h>
#include<stdint.h>
#ifndef ALLOCATOR_H
#define ALLOCATOR_H

typedef struct node {
    int16_t size;
    struct node * next;
} node_t;

void initialize_allocator(int heap_size);

void * my_malloc(int size);

void my_free(void * address);

void print_free_list();
#endif