#include "allocator.h"

//allocate memory on stack
static char heap[HEAP_SIZE];

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

void initialize_allocator() {
    node_t * new_free_list = (node_t *) heap;
    new_free_list -> size = HEAP_SIZE - sizeof(node_t);
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
        initialize_allocator();
    } else {
        node_t * new_node = (node_t*) address - 1;
        new_node -> next = free_list;
        free_list = new_node;
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

int main() {
    initialize_allocator();
    print_free_list();

    //test simple allocation
    printf("allocating for a\n");
    int * a = my_malloc(sizeof(int));
    *a = 3;
    print_free_list();
    printf("a = %d\n", *a);


    //test free

    printf("freeing a\n");
    my_free(a);
    print_free_list();

    //test second allocation
    printf("allocating for b\n");
    int * b = my_malloc(sizeof(int));
    *b = 5;
    print_free_list();
    printf("b = %d\n", *b);
}