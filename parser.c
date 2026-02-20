#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include "parser.h"
#include "allocator.h"

static void * variables[5];

void parse_malloc(char * s) {
    strtok(s, " ");//discard "malloc" operator
    char * name_string = strtok(NULL, " ");//variable name
    char * size_string = strtok(NULL, " ");//size
    char name = *name_string;
    int size = atoi(size_string);

    printf("%c = malloc(%d)\n", name, size);
    void * allocated = my_malloc(size);

    int index = name - 'a';
    variables[index] = allocated;
}

void parse_free(char * s) {
    strtok(s, " ");//discoard "free" operator
    char * name_string = strtok(NULL, " ");//name of variable to free
    char name = *name_string;

    int index = name - 'a';
    void * allocated = variables[index];
    printf("free(%c)\n", name);
    my_free(allocated);
}

void parse_set(char * s) {
    strtok(s, " ");//discard "set" operator
    char * name_string = strtok(NULL, " ");//name of variable to set
    char * value_string = strtok(NULL, " ");//value to set variable to
    int value = atoi(value_string);
    char name = *name_string;

    int index = name - 'a';
    printf("%c = %d\n", name, value);
    *((int *)variables[index]) = value;
}

void parse_print(char * s) {
    strtok(s, " ");//discard "print" operator
    char * operand = strtok(NULL, " ");//either variable name of memory
    if (strlen(operand) == 2) {//variable name + \n
        //variable name case
        char variable = *operand;
        int index = variable - 'a';
        if (variables[index] == NULL) {
            printf("NULL Pointer\n");
        } else {
            int value = *((int *)variables[index]);
            printf("%c == %d\n", variable, value);
        }
    } else {
        //print memory case
        print_free_list();
    }
}

void parse_initialize(char * s) {
    strtok(s, " ");//discard "initialize" operator
    char * size_string = strtok(NULL, " ");
    int size = atoi(size_string);
    printf("Initializing allocator with %d bytes\n", size);
    initialize_allocator(size);
}

void parse() {
    char buffer[50];
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (strncmp("malloc", buffer, 6) == 0) {
            parse_malloc(buffer);
        } else if (strncmp("free", buffer, 4) == 0) {
            parse_free(buffer);
        } else if (strncmp("print", buffer, 5) == 0) {
            parse_print(buffer);
        } else if (strncmp("set", buffer, 3) == 0) {
            parse_set(buffer);
        } else if (strncmp("initialize", buffer, 10) == 0) {
            parse_initialize(buffer);
        } else {
            printf("Unknown command: %s\n", buffer);
        }
    }
}

int main() {
    parse();
}