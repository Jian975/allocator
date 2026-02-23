#include<stdlib.h>
#include "allocator.h"

//allocate memory
static char * heap = NULL;
static node_t metadata[METADATA_SIZE];
static int metadata_size = 1;
static int heap_size = 0;
static int8_t free_list = -1;
//if free list is empty
static int is_empty = 0;

static int delta(int a, int b) {
    int difference = a - b;
    if (difference < 0) {
        return -difference;
    }
    return difference;
}

//shift all values to the right of i (excluding i) by 1 to the right
static void shift_right(int i) {
    for (int j = metadata_size; j > i + 1; j--) {
        metadata[j] = metadata[j - 1];
    }
}

static int find(void * address) {
    for (int i = 0; i < metadata_size; i++) {
        if (metadata[i].address == address) {
            return i;
        }
    }
    return -1;
}
//shift all values to the right of i (excluding i) by 1
static void shift_left(int i) {
    for (int j = i + 1; j < metadata_size - 1; j++) {
        metadata[j] = metadata[j + 1];
    }
}

void initialize_allocator(int new_heap_size) {
    heap_size = new_heap_size;
    if (heap == NULL) {
        heap = malloc(heap_size);
    }
    metadata[0].allocated = 0;
    metadata[0].next_free = -1;
    metadata[0].size = heap_size;
    metadata[0].address = heap;
    free_list = 0;
}


void * my_malloc(int size) {
    if (is_empty) {
        return NULL;
    }
    //best fit
    int8_t current = free_list;
    int8_t previous = -1;
    int smallest_delta = -1;
    int8_t best_fit = -1;
    int8_t before_best_fit = -1;//keep track for splitting memory chunks
    while (current != -1) {
        int current_delta = delta(metadata[current].size, size);
        if (metadata[current].size >= size && (smallest_delta == -1 || current_delta < smallest_delta)) {
            smallest_delta = current_delta;
            best_fit = current;
            before_best_fit = previous;
        }
        previous = current;
        current = metadata[current].next_free;
    }

    //out of memory
    if (best_fit == -1) {
        return NULL;
    }

    //split memory block if necessary
    if (metadata[best_fit].size > size) {
        shift_right(best_fit);
        metadata_size++;
        node_t * new_node = &metadata[best_fit + 1];
        new_node -> next_free = metadata[best_fit].next_free;
        new_node -> allocated = 0;
        new_node -> address = (char *) metadata[best_fit].address + size;
        new_node -> size = delta(metadata[best_fit].size, size);
        if (best_fit + 2 == metadata_size) {
            new_node -> next_free = -1;
        }
        if (before_best_fit != -1) {
            metadata[before_best_fit].next_free = best_fit + 1;
        } else{//no previous means we removed first node
            free_list = best_fit + 1;
        }
        metadata[best_fit].size = size;
    } else {
        if (before_best_fit != -1) {
            metadata[before_best_fit].next_free = metadata[best_fit].next_free;
            metadata[best_fit].size = size;
        } else {
            //We removed the only entry and we didn't split any memory chunks
            //free list is now empty
            is_empty = 1;
        }
    }

    metadata[best_fit].allocated = 1;
    return metadata[best_fit].address;
}

void my_free(void * address) {
    if (address == NULL) {
        return;
    }
    if (is_empty) {
        initialize_allocator(heap_size);
    } else {
        int8_t last_free = -1;
        for (int i = 0; i < metadata_size; i++) {
            if (metadata[i].allocated == 0) {
                last_free = i;
            }
            if (metadata[i].address == address) {
                metadata[i].allocated = 0;
                if (i > 1 && metadata[i - 1].allocated == 0) {
                    metadata[i - 1].size += metadata[i].size;
                    metadata[i - 1].next_free = metadata[i].next_free;
                    shift_left(i - 1);
                    metadata_size--;
                    i--;
                }
                if (metadata[i + 1].allocated == 0) {
                    metadata[i].size += metadata[i + 1].size;
                    metadata[i].next_free = metadata[i + 1].next_free;
                    if (last_free == i + 1) {
                        metadata[last_free].next_free = i;
                    }
                    shift_left(i);
                    metadata_size--;
                    break;
                }
            }
        }
    }
    is_empty = 0;
}

void print_memory() {
    for (int i = 0; i < metadata_size; i++) {
        ptrdiff_t relative_address = (uintptr_t) metadata[i].address - (uintptr_t) heap;
        printf("[size=%d, allocated=%d, address=%p]\n", 
            metadata[i].size, metadata[i].allocated, relative_address);
    }
}

// int main() {
//     initialize_allocator(1024);
//     print_memory();

//     //test simple allocation
//     printf("allocating for a\n");
//     int * a = my_malloc(sizeof(int));
//     *a = 3;
//     print_memory();
//     printf("a = %d\n", *a);

//     //test second allocation
//     printf("allocating for b\n");
//     int * b = my_malloc(sizeof(int));
//     *b = 5;
//     print_memory();
//     printf("b = %d\n", *b);

//     //test second allocation
//     printf("allocating for c\n");
//     int * c = my_malloc(sizeof(int));
//     *c = 18;
//     print_memory();
//     printf("c = %d\n", *c);

//     //test free
//     printf("freeing b\n");
//     my_free(b);
//     print_memory();

//     //test free
//     printf("freeing c\n");
//     my_free(c);
//     print_memory();

//     //test second allocation
//     // printf("allocating for b\n");
//     // int * b = my_malloc(sizeof(int));
//     // *b = 5;
//     // print_free_list();
//     // printf("b = %d\n", *b);
// }