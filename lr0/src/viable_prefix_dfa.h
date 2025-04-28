#ifndef VIABLE_PREFIX_DFA_H
#define VIABLE_PREFIX_DFA_H

#include "grammar.h"
#include "arena.h"
#include <stdint.h>


typedef struct DFAItem
{
    const char* restrict right_symbols;
    size_t right_len;
    char left_symbol;
    uint8_t dot;
} DFAItem;

typedef struct ItemSet
{
    DFAItem* items;
    uint8_t item_count;
    uint8_t item_capacity;
} ItemSet;

typedef struct Transition{
    uint8_t from_state;
    char symbol;
    uint8_t to_state;
} Transition;

typedef struct DFA{
    ItemSet* states;
    uint8_t state_count;
    uint8_t state_capacity;
    Transition* transitions;
    uint16_t transition_count;
    uint16_t transition_capacity;
    Arena* arena;
} DFA;

void build_viable_prefix_dfa(const Grammar* grammar, DFA* dfa, Arena* arena);
void dfa_free(DFA* dfa);
void dfa_export_dot(const DFA* dfa, const char* filename);
void print_dfa(const DFA* dfa);

#endif