#include<stdio.h>
#include "allocator.h"
#ifndef PARSER_H
#define PARSER_H

//string format: m <var name a-z> <size in bytes>
void parse_malloc(char * s);

//string format: f <var name a-z>
void parse_free(char *s);

//string format: p
void parse_print(char *s);

//string format: <number of bytes to give allocator>
void parse_initialize(char * s);

//string format: g <var name a-z>
void parse_get(char * s);

//string format: s <var name a-z> <int value>
void parse_set(char * s);

//takes in a series of commands from stdin and calls appropriate parser
void parse();
#endif