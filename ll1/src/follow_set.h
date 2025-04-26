#ifndef FOLLOW_SET_H
#define FOLLOW_SET_H

#include "arena.h"
#include "grammar.h"
#include "first_follow.h"

#include <string.h>
#include <stdbool.h>


void compute_follow_sets(Grammar* grammar, SymbolSet* sets, int* set_count, Arena* arena);


#endif