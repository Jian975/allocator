#include<stdlib.h>
#include<stdio.h>
#include "free_list_allocator.h"

int delta(int size, int target_size) {
    int distance = target_size - size;
    if (distance > 0) {
        return distance;
    }
    return -distance;
}

int can_fit(int size, int target_size) {
    return size >= target_size;
}

int within(int lower, int upper, int n) {
    return n >= lower && n <= upper;
}

void * best_fit(free_list_t * free_list, int target_size) {
    int smallest_delta = -1;
    char * best_location = NULL;

    node_t * node = free_list -> head;
    //out of memory
    if (node == NULL) {
        return NULL;
    }
    for (int i = 0; i < free_list -> size; i++) {
        if (can_fit(node -> size, target_size)) {
            int current_delta = delta(node -> size, target_size);
            if (smallest_delta == -1 || current_delta < smallest_delta) {
                smallest_delta = current_delta;
                best_location = node -> data;
            }
        }
    }
    if (smallest_delta > 0) {
        add_node(free_list, best_location + target_size, smallest_delta);
    }
    remove_node(free_list, best_location);
    return best_location;
}

void remove_node(free_list_t * free_list, void * address) {
    node_t * current = free_list -> head;
    if (current -> data == address) {
        free_list -> head = current -> next;
        free_list -> size -= 1;
        return;
    }

    for (int i = 0; i < free_list -> size; i++) {
        node_t * previous = current;
        node_t * current = previous -> next;
        if (current -> data == address) {
            previous -> next = current -> next;
            free_list -> size -= 1;
            return;
        }
    }
}

//prepend
void add_node(free_list_t * free_list, void * address, int size) {
    node_t * node = malloc(sizeof(node_t));
    node -> data = address;
    node -> size = size;
    node -> next = free_list -> head;
    free_list -> head = node;
    free_list -> size += 1;
}

void * block_allocate(block_t * block, int size) {
    return best_fit(block -> free_list, size);
}

void block_free(block_t * block, void * address) {
    int size = 0;
    add_node(block -> free_list, address, size);
    if (block -> free_list -> size == 1) {
        block -> free_list -> head -> size = BLOCK_SIZE;
    } else {
        size = front_merge(block -> free_list, address, size);
        back_merge(block -> free_list, address, size);
    }
}

int front_merge(free_list_t * free_list, void * address, int address_size) {
    //nothing to merge
    if (free_list -> size == 1) {
        return address_size;
    }
    node_t * current = free_list -> head;
    int merged_size = address_size;
    while (current != NULL) {
        char * data = current -> data;
        int current_size = current -> size;
        char * data_end = data + current_size;
        if (data_end == address) {
            merged_size += current_size;
            current -> size = merged_size;
            break;
        }
        current = current -> next;
    }

    remove_node(free_list, address);

    return merged_size;
}

int back_merge(free_list_t * free_list, void * address, int address_size) {
    //nothing to merge
    if (free_list -> size == 1) {
        return address_size;
    }
    char * address_end = (char *) address + address_size;
    node_t * current = free_list -> head;
    int merged_size = address_size;
    while (current != NULL) {
        if ((char *) current -> data == address_end) {
            merged_size += current -> size;
            break;
        }
        current = current -> next;
    }

    if (merged_size > address_size) {
        update_size(free_list, address, merged_size);
        remove_node(free_list, address_end);
    }
}

void update_size(free_list_t * free_list, void * address, int new_size) {
    node_t * current = free_list -> head;
    while (current != NULL) {
        if (current -> data == address) {
            current -> size = new_size;
            return;
        }
        current = current -> next;
    }
}

block_t * initialize_block() {
    void * data = malloc(BLOCK_SIZE);
    node_t * node = malloc(sizeof(node_t));
    node -> data = data;
    node -> size = BLOCK_SIZE;
    node -> next = NULL;

    free_list_t * free_list = malloc(sizeof(free_list_t));
    free_list -> size = 1;
    free_list -> head = node;

    block_t * block = malloc(sizeof(block_t));
    block -> data = data;//todo do we even need this?
    block -> free_list = free_list;

    return block;
}

void print_free_list(free_list_t * free_list) {
    printf("free list:\n");
    node_t * current = free_list -> head;
    while (current != NULL) {
        printf("\tAddress: %p, Size: %d\n", current -> data, current -> size);
        current = current -> next;
    }
}

int main() {
    block_t * block = initialize_block();
    print_free_list(block -> free_list);

    int * a = block_allocate(block, sizeof(int));
    *a = 3;
    print_free_list(block -> free_list);
    int * b = block_allocate(block, sizeof(int));
    *b = 999;
    print_free_list(block -> free_list);

    int * c = block_allocate(block, sizeof(int));
    print_free_list(block -> free_list);
    if (c != NULL) {
        printf("C is not null!");
    }

    printf("a = %d\n", *a);
    printf("b = %d\n", *b);

    block_free(block, a);
    print_free_list(block -> free_list);

    int * d = block_allocate(block, sizeof(int));
    if (d == NULL) {
        printf("D is shouldn't be null!");
        return 0;
    }
    *d = 3;

    printf("b = %d\n", *b);
    printf("d = %d\n", *d);

    return 0;
}