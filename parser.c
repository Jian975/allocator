#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include "parser.h"
#include "allocator.h"

static void * variables[26];

void parse_malloc(char * s) {
    char name = s[2];
    char * size_string = s + 4;
    int size = atoi(size_string);

    printf("%c = malloc(%d)\n", name, size);
    void * allocated = my_malloc(size);

    int index = name - 'a';
    variables[index] = allocated;
}

void parse_free(char * s) {
    char name = s[2];

    int index = name - 'a';
    void * allocated = variables[index];
    printf("free(%c)\n", name);
    my_free(allocated);
}

void parse_set(char * s) {
    char name = s[2];
    char * value_string = s + 4;
    int value = atoi(value_string);

    int index = name - 'a';
    printf("Setting %c = %d\n", name, value);
    *((int *)variables[index]) = value;
}

void parse_print(char * s) {
    print_memory();
}

void parse_initialize(char * s) {
    int size = atoi(s);
    printf("Initializing allocator with %d bytes\n", size);
    initialize_allocator(size);
}

void parse_get(char * s) {
    char name = s[2];
    int index = name - 'a';
    printf("Value of %c: %d\n", name, *((int *)variables[index]));
}

void parse() {
    char buffer[50];
    fgets(buffer, sizeof(buffer), stdin);
    parse_initialize(buffer);
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (buffer[0] == 'm') {
            parse_malloc(buffer);
        } else if (buffer[0] == 'f') {
            parse_free(buffer);
        } else if (buffer[0] == 'p') {
            parse_print(buffer);
        } else if (buffer[0] == 's') {
            parse_set(buffer);
        } else if (buffer[0] == 'g') {
            parse_get(buffer);
        } else {
            printf("Unknown command: %s\n", buffer);
        }
    }
}

int main() {
    parse();
}