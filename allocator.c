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
    heap_size = new_heap_size;
    if (heap == NULL) {
        heap = malloc(heap_size);
    }
    metadata[0].allocated = 0;
    metadata[0].size = heap_size;
    metadata[0].address = heap;
}


void * my_malloc(int size) {
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
        return NULL;
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

void * my_realloc(void * ptr, size_t size) {
    char * address = my_malloc(size);
    if (address == NULL) {
        return NULL;
    }
    if (ptr == NULL) {
        return address;
    }
    if (size == 0) {
        my_free(ptr);
        return NULL;
    }
    char * address_start = address;
    node_t * metadata = (node_t * ) ((char *) ptr - sizeof(node_t));
    char * char_ptr = (char *) ptr;
    for (int i = 0; i < minimum(metadata -> size, size); i++) {
        *address = *char_ptr;
        address++;
        char_ptr++;
    }
    my_free(ptr);
    return address_start;
}

static void *my_calloc(size_t nmemb, size_t size) {
    char * ptr = my_malloc(nmemb * size);
    for (size_t i = 0; i < (nmemb * size); i++) {
        *ptr = '\0';
        ptr++;
    }
    return ptr;
}

void my_free(void * address) {
    if (address == NULL) {
        return;
    }
    if (metadata_size == 1) {
        initialize_allocator(heap_size);
        return;
    }
    int8_t freed = find(address);
    if (freed == -1) {
        printf("Error: Can't find address to free\n");
        return;
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
        ptrdiff_t relative_address = (uintptr_t) metadata[i].address - (uintptr_t) heap;
        printf("[size=%d, allocated=%d, address=%p]\n", 
            metadata[i].size, metadata[i].allocated, relative_address);
    }
    printf("\n");
}

// int main() {
//     initialize_allocator(1024);
//     print_memory();

//     void * c = my_malloc(SIZE_MAX);
//     my_free(c);
//     print_memory();
//     int * a = my_malloc(sizeof(int));
//     print_memory();
//     int * b = my_realloc(a, 5);
//     print_memory();

    // //test simple allocation
    // printf("allocating for a\n");
    // int * a = my_malloc(sizeof(int));
    // *a = 3;
    // print_memory();
    // printf("a = %d\n", *a);

    // //test second allocation
    // printf("allocating for b\n");
    // int * b = my_malloc(sizeof(int));
    // *b = 5;
    // print_memory();
    // printf("b = %d\n", *b);

    // //test second allocation
    // printf("allocating for c\n");
    // int * c = my_malloc(sizeof(int));
    // *c = 18;
    // print_memory();
    // printf("c = %d\n", *c);

    // //test free
    // printf("freeing b\n");
    // my_free(b);
    // print_memory();

    // //test free
    // printf("freeing c\n");
    // my_free(c);
    // print_memory();

    //test second allocation
    // printf("allocating for b\n");
    // int * b = my_malloc(sizeof(int));
    // *b = 5;
    // print_free_list();
    // printf("b = %d\n", *b);
// }