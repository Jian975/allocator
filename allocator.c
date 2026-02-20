#include<stdlib.h>
#include "allocator.h"

//allocate memory
static char * heap = NULL;
static int heap_size = 0;
static node_t * free_list = NULL;
//if free list is empty
static int is_empty = 0;

static int delta(int a, int b) {
    int difference = a - b;
    if (difference < 0) {
        return -difference;
    }
    return difference;
}

//given that first node goes before next node
//return 0 if the two nodes are not touching
static int can_merge(node_t * first, node_t * next) {
    return ((char*) (first + 1) + first -> size) == next;
}

//merges eaten into eater and returns pointer to eater
static node_t * merge(node_t * eater, node_t * eaten) {
    eater -> size += (eaten -> size + sizeof(node_t));
    eater -> next = eaten -> next;
    return eater;
}

void initialize_allocator(int new_heap_size) {
    heap_size = new_heap_size;
    if (heap == NULL) {
        heap = malloc(heap_size);
    }
    node_t * new_free_list = (node_t *) heap;
    new_free_list -> size = heap_size - sizeof(node_t);
    new_free_list -> next = NULL;
    free_list = new_free_list;
}


void * my_malloc(int size) {
    if (is_empty) {
        return NULL;
    }
    //best fit
    node_t * current = free_list;
    node_t * previous = NULL;
    int smallest_delta = -1;
    node_t * best_fit = NULL;
    node_t * before_best_fit = NULL;//keep track for splitting memory chunks
    while (current != NULL) {
        int current_delta = delta(current -> size, size);
        if (current -> size >= size && (smallest_delta == -1 || current_delta < smallest_delta)) {
            smallest_delta = current_delta;
            best_fit = current;
            before_best_fit = previous;
        }
        previous = current;
        current = current -> next;
    }

    //out of memory
    if (best_fit == NULL) {
        return NULL;
    }

    //split memory block if necessary
    if (best_fit -> size > size + sizeof(node_t)) {
        node_t * new_node = ((char*) (best_fit + 1)) + size;
        new_node -> next = best_fit -> next;
        if (before_best_fit != NULL) {
            before_best_fit -> next = new_node;
        } else{//no previous means we removed first node
            free_list = new_node;
        }
        new_node -> size = delta(best_fit -> size, size + sizeof(node_t));
        best_fit -> size = (char*) new_node - (char*) best_fit - sizeof(node_t);
    } else {
        if (before_best_fit != NULL) {
            before_best_fit -> next = best_fit -> next;
            best_fit -> size = (char*) best_fit -> next - (char*) best_fit - sizeof(node_t);
        } else {
            //We removed the only entry and we didn't split any memory chunks
            //free list is now empty
            is_empty = 1;
        }
    }

    return best_fit + 1;
}

void my_free(void * address) {
    if (address == NULL) {
        return;
    }
    if (is_empty) {
        initialize_allocator(heap_size);
    } else {
        node_t * new_node = (node_t*) address - 1;
        node_t * current = free_list;
        node_t * previous = NULL;
        while (current != NULL) {
            if (current > new_node) {
                //freed block is first chunk in memory, with one or more following it
                if (previous == NULL) {
                    //merge freed block with next block if we can merge
                    if (can_merge(new_node, current)) {
                        new_node = merge(new_node, current);
                        free_list = new_node;
                    } else {
                        //can't merge, just insert into linked list
                        new_node -> next = current;
                        free_list = new_node;
                    }
                    
                } else {
                    //freed block is not first chunk in memory
                    previous -> next = new_node;
                    new_node -> next = current;
                    if (can_merge(previous, new_node)) {
                        new_node = merge(previous, new_node);
                    }
                    if (can_merge(new_node, current)) {
                        merge(new_node, current);
                    }
                }
                break;
            }
            previous = current;
            current = current -> next;
        }
    }
    is_empty = 0;
}

void print_free_list() {
    if (is_empty) {
        printf("Empty free list\n");
        return;
    }
    node_t * current = free_list;
    while (current != NULL) {
        printf("[size=%d, addr=%p]", current -> size, current + 1);
        current = current -> next;
    }
    printf("\n");
}

// int main() {
//     initialize_allocator(1024);
//     print_free_list();

//     //test simple allocation
//     printf("allocating for a\n");
//     int * a = my_malloc(sizeof(int));
//     *a = 3;
//     print_free_list();
//     printf("a = %d\n", *a);

//     //test second allocation
//     printf("allocating for b\n");
//     int * b = my_malloc(sizeof(int));
//     *b = 5;
//     print_free_list();
//     printf("b = %d\n", *b);

//     //test second allocation
//     printf("allocating for c\n");
//     int * c = my_malloc(sizeof(int));
//     *c = 18;
//     print_free_list();
//     printf("c = %d\n", *c);

//     //test free
//     printf("freeing b\n");
//     my_free(b);
//     print_free_list();

//     //test free
//     printf("freeing c\n");
//     my_free(c);
//     print_free_list();

//     //test second allocation
//     // printf("allocating for b\n");
//     // int * b = my_malloc(sizeof(int));
//     // *b = 5;
//     // print_free_list();
//     // printf("b = %d\n", *b);
// }