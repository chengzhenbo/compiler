// nfa.h
#ifndef NFA_H
#define NFA_H

#include "arena.h"

typedef struct State State;
typedef struct Transition Transition;

struct Transition {
    char symbol; // '\0' for epsilon
    State* target;
    Transition* next;
};

struct State {
    int is_accepting;
    Transition* transitions;
};

typedef struct {
    State* start;
    State* accept;
} NFA;

State* create_state(struct Arena* arena);
void add_transition(struct Arena* arena, State* from, char symbol, State* to);
NFA create_char_nfa(struct Arena* arena, char c);
NFA create_concat_nfa(struct Arena* arena, NFA a, NFA b);
NFA create_star_nfa(struct Arena* arena, NFA inner);

#endif