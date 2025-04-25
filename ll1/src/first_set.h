#ifndef FIRST_SET_H
#define FIRST_SET_H

#include "arena.h"
#include "grammar.h"

typedef struct SymbolSet{
    char symbol;
    char* first;
    char* follow;
} SymbolSet;

void compute_first_sets(Grammar* grammar, SymbolSet* sets, int* set_count, Arena* arena);

#endif