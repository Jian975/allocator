#define BLOCK_SIZE 8

typedef struct Node {
    void * data;
    int size;
    int allocated;//1 if allocated, 0 if not
    struct Node * next;
} node_t;

typedef struct {
    node_t * head;
    int size;
} memory_list_t;

typedef struct {
    void * data;
    memory_list_t * memory_list;
} block_t;

void * best_fit(memory_list_t * memory_list, int size);

void remove_node(memory_list_t * memory_list, void * address);

block_t * initialize_block();

void * block_allocate(block_t * block, int size);

void block_free(block_t * block, void * address);

void add_node(memory_list_t * memory_list, void * address, int size);

//merge address with the memory chunk in front if possible
//returns the size of the merged memory chunk
int front_merge(memory_list_t * memory_list, void * address, int size);

//merge address with the memory chunk beyond address if possible
//returns the size of the merged memory chunk
int back_merge(memory_list_t * memory_list, void * address, int size);

void update_size(memory_list_t * memory_list, void * address, int new_size);

void print_memory_list(memory_list_t * memory_list);