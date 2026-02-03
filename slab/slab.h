#ifndef SLAB_H
#define SLAB_H
#define SLAB_SIZE 2

typedef struct free_list{
    struct free_list * next;
    void * data;
} free_list_t;

typedef struct {
    free_list_t * free_list;
    void * data;
    int element_size;
    int size;
} slab_t;

//allocate a single block of a fixed size
void * my_malloc(slab_t * slab);

//add data pointer to free list
void my_free(slab_t * slab, void * data);

//initiates a slab of SLAB_SIZE elements with given element size
slab_t * initiate_slab(int element_size);
#endif