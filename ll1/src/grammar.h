#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "arena.h"
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_SYMBOLS 64
#define MAX_RULES 128

typedef struct Rule
{
    char left_hs;   //产生式左边的非终结符
    char* right_hs; //产生式右边的串，包括终结符与非终结符
} Rule;

typedef struct Grammar
{
    Rule* rules;
    int rule_count;
    char nonderminals[MAX_SYMBOLS];
    int nondermials_count;
    char derminals[MAX_SYMBOLS];
    int dermials_count;
} Grammar;

Grammar* read_grammar(const char* filename, Arena* arena);

#endif