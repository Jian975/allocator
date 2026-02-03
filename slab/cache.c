#include<stdlib.h>
#include<stdio.h>
#include "cache.h"
#include "slab.h"

cache_t * initiate_cache() {
    cache_t * cache = malloc(sizeof(cache_t));
    cache -> slab = NULL;
    cache -> next = NULL;
    cache_t * head = cache;
    for (int i = 0; i < CACHE_SIZE; i++) {
    }
}

int power(int b, int n) {
    int output = b;
    for (int i = 0; i < n; i++) {
        output *= b;
    }
}

int to_index(int size) {
    int counter = 0;
    float float_size = size;
    while (float_size > 1.0) {
        float_size = float_size / 2.0;
        counter++;
    }
    return counter;
}

// int main() {
//     int idx = power(2, 3);

//     printf("index = %d\n", idx);
// }