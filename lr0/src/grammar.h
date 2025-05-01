#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "arena.h"

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define MAX_SYMBOLS 64
#define MAX_RULES 128

typedef struct Rule
{
    char left_hs;   //产生式左边的非终结符
    char* right_hs; //产生式右边的串，包括终结符与非终结符
    uint8_t right_hs_count;
} Rule;

typedef struct Grammar
{
    Rule* rules;
    uint8_t rule_count;
    char nonterminals[MAX_SYMBOLS];
    uint8_t nonterminals_count;
    char terminals[MAX_SYMBOLS];
    uint8_t terminals_count;
} Grammar;

Grammar* read_grammar(const char* filename, Arena* arena);
void grammar_free(Grammar* grammar);

#endif