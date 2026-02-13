#include<stdio.h>
#include<stdint.h>
#define HEAP_SIZE 1024

typedef struct node {
    int16_t size;
    struct node * next;
} node_t;

void initialize_allocator();

void * my_malloc(int size);

void my_free(void * address);

void print_free_list();