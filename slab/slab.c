#include<stdlib.h>
#include<stdio.h>
#include "slab.h"

//allocated a fixed size block
void * slab_malloc(slab_t * slab) {
    free_list_t * free_list = slab -> free_list;
    if (free_list == NULL) {
        printf("Out of memory");
        return NULL;
    }
    free_list_t * next = free_list -> next;
    void * allocated = free_list -> data;
    free(free_list);
    slab -> free_list = next;
    return allocated;
}

//prepend the pointer of the freed data to the start of the free list
void slab_free(slab_t * slab, void * data) {
    free_list_t * free_list = slab -> free_list;
    free_list_t * freed = malloc(sizeof(free_list_t));
    freed -> data = data;
    freed -> next = free_list;
    slab -> free_list = freed;
}

//initiate a slab. This slab can only provide elements of size element_size
slab_t * initiate_slab(int element_size) {
    slab_t * slab = malloc(sizeof(slab_t));

    slab -> element_size = element_size;
    slab -> size = SLAB_SIZE;
    char *  data = malloc(element_size * SLAB_SIZE);
    slab -> data = data;

    free_list_t * free_list = malloc(sizeof(free_list_t));
    free_list -> next = NULL;
    free_list_t * head = free_list;
    for (int i = 0; i < SLAB_SIZE; i++) {
        free_list -> data = data + (i * element_size);
        if (i < SLAB_SIZE - 1) {
            free_list_t * next_node = malloc(sizeof(free_list_t));
            next_node -> next = NULL;
            free_list -> next = next_node;
            free_list = free_list -> next;
        } else {
            free_list -> next = NULL;
        }
    }

    slab -> free_list = head;

    return slab;
}

int main() {
    slab_t * slab = initiate_slab(sizeof(int));

    int * a = slab_malloc(slab);
    *a = 10;

    int * b = slab_malloc(slab);
    *b = 20;
    
    int * c = slab_malloc(slab);
    if (c != NULL) {
        printf("ERROR: We should have been out of memory by now");
    }

    printf("a = %d\n", *a);
    printf("b = %d\n", *b);

    slab_free(slab, a);

    int * d = slab_malloc(slab);
    *d = 30;

    printf("b = %d\n", *b);
    printf("d = %d\n", *d);

    int * e = slab_malloc(slab);
    if (e != NULL) {
        printf("ERROR: We should have been out of memory by now");
    }
}