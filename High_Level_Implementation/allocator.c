#include<stdlib.h>
#include "allocator.h"

//allocate memory
static char * heap = NULL;
static node_t metadata[METADATA_SIZE];
static int metadata_size = 1;
static int heap_size = 0;

static int delta(int a, int b) {
    int difference = a - b;
    if (difference < 0) {
        return -difference;
    }
    return difference;
}

static int minimum(int a, int b) {
    if (a < b) {
        return a;
    }
    return b;
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
    if (new_heap_size < 4 || new_heap_size > 1000) {
        printf("Invalid arena size, allocator terminating.\n");
        exit(0);
    }
    heap_size = new_heap_size;
    if (heap == NULL) {
        heap = malloc(heap_size);
    }
    metadata[0].allocated = 0;
    metadata[0].size = heap_size;
    metadata[0].address = heap;
}


void * my_malloc(int size) {
    if (size % 4 != 0) {
        size += (4 - (size % 4));
    }
    //best fit
    int8_t current = 0;
    int smallest_delta = -1;
    int8_t best_fit = -1;
    while (current < metadata_size) {
        int current_delta = delta(metadata[current].size, size);
        if (metadata[current].allocated == 0 &&
            metadata[current].size >= size && 
            (smallest_delta == -1 || current_delta < smallest_delta)) {
            smallest_delta = current_delta;
            best_fit = current;
        }
        current += 1;
    }
    //out of memory
    if (best_fit == -1) {
        printf("Out of Memory\n");
        exit(1);
    }
    //split memory block if necessary
    if (metadata[best_fit].size > size) {
        shift_right(best_fit);
        metadata_size++;
        node_t * new_node = &metadata[best_fit + 1];
        new_node -> allocated = 0;
        new_node -> address = (char *) metadata[best_fit].address + size;
        new_node -> size = delta(metadata[best_fit].size, size);
        metadata[best_fit].size = size;
    }
    metadata[best_fit].allocated = 1;
    return metadata[best_fit].address;
}

void my_free(void * address) {
    if (address == NULL) {
        printf("Freeing memory not from malloc\n");
        exit(1);
    }
    int8_t freed = find(address);
    if (freed == -1) {
        printf("Freeing memory not from malloc\n");
        exit(1);
    }
    //set deallocated memory to 0 (unnecessary)
    char * char_address = address;
    for (int i = 0; i < metadata[freed].size; i++) {
        *char_address = '\0';
        char_address++;
    }
    metadata[freed].allocated = 0;
    //if the block after this is free, coalesce
    if (freed < metadata_size - 1 && metadata[freed + 1].allocated == 0) {
        metadata[freed].size += metadata[freed + 1].size;
        shift_left(freed);
        metadata_size--;
    }
    //if the block before this is free, coalesce
    if (freed > 0 && metadata[freed - 1].allocated == 0) {
        metadata[freed - 1].size += metadata[freed].size;
        shift_left(freed - 1);
        metadata_size--;
    }
}

void print_memory() {
    for (int i = 0; i < metadata_size; i++) {
        int relative_address = (int) ((uintptr_t) metadata[i].address - (uintptr_t) heap);
        printf("[size=%d, allocated=%d, address=%d]\n", 
            metadata[i].size, metadata[i].allocated, relative_address);
    }
}