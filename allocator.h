#include<stdio.h>
#define HEAP_SIZE 37

typedef struct node {
    int size;
    struct node * next;
} node_t;

void initialize_allocator();

void * my_malloc(int size);

void my_free(void * address);

void print_free_list();