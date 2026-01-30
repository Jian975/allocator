#include<stdlib.h>
#include<stdio.h>
#include "slab.h"

//allocated a fixed size block
void * my_malloc(slab_t * slab) {
    free_list_t * free_list = slab -> free_list;
    if (free_list == NULL) {
        printf("Out of memory");
        return NULL;
    }
    free_list_t * next = free_list -> next;
    void * allocated = free_list -> data;
    slab -> free_list = next;
    return allocated;
}

//prepend the pointer of the freed data to the start of the free list
void my_free(slab_t * slab, void * data) {
    free_list_t * free_list = slab -> free_list;
    free_list_t * freed = malloc(sizeof(free_list_t));
    freed -> data = data;
    freed -> next = free_list;
    slab -> free_list = freed;
}

//initiate a slab. This slab can only provide elements of size element_size
slab_t * initiate(int element_size) {
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
    slab_t * slab = initiate(sizeof(int));

    int * a = my_malloc(slab);
    *a = 10;

    int * b = my_malloc(slab);
    *b = 20;
    
    int * c = my_malloc(slab);
    if (c != NULL) {
        printf("ERROR: We should have been out of memory by now");
    }

    printf("a = %d\n", *a);
    printf("b = %d\n", *b);

    my_free(slab, a);

    int * d = my_malloc(slab);
    *d = 30;

    printf("b = %d\n", *b);
    printf("d = %d\n", *d);

    int * e = my_malloc(slab);
    if (e != NULL) {
        printf("ERROR: We should have been out of memory by now");
    }


}