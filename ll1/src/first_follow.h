#ifndef FIRST_FOLLOW_H
#define FIRST_FOLLOW_H

#include "arena.h"
#include "grammar.h"

#include <stdbool.h>


typedef struct SymbolSet{
    char symbol;
    char* first;
    char* follow;
} SymbolSet;

SymbolSet* get_or_create_set(SymbolSet* sets, int* count, char symbol, Arena* arena);
bool add_char(char* set, char c);
bool is_terminal(char c);
bool is_nonterminal(char c);

#endif