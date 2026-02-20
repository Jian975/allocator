#include<stdio.h>
#include "allocator.h"
#ifndef PARSER_H
#define PARSER_H

//string format: malloc <var name a-e> <size in bytes>
void parse_malloc(char * s);

//string format: free <var name a-e>
void parse_free(char *s);

//string format: print <var name a-e | memory>
void parse_print(char *s);

//string format: <number of bytes to give allocator>
void parse_initialize(char * s);

//takes in a series of commands from stdin and calls appropriate parser
void parse();
#endif