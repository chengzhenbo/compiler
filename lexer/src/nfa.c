// nfa.c
#include "nfa.h"
#include <stdlib.h>

State* create_state(struct Arena* arena) {
    State* s = (State*)arena_alloc(arena, sizeof(State));
    s->is_accepting = 0;
    s->transitions = NULL;
    return s;
}

void add_transition(struct Arena* arena, State* from, char symbol, State* to) {
    Transition* t = (Transition*)arena_alloc(arena, sizeof(Transition));
    t->symbol = symbol;
    t->target = to;
    t->next = from->transitions;
    from->transitions = t;
}

NFA create_char_nfa(struct Arena* arena, char c) {
    State* start = create_state(arena);
    State* accept = create_state(arena);
    add_transition(arena, start, c, accept);
    accept->is_accepting = 1;
    return (NFA){start, accept};
}

NFA create_concat_nfa(struct Arena* arena, NFA a, NFA b) {
    add_transition(arena, a.accept, '\0', b.start);
    a.accept->is_accepting = 0;
    return (NFA){a.start, b.accept};
}

NFA create_star_nfa(struct Arena* arena, NFA inner) {
    State* start = create_state(arena);
    State* accept = create_state(arena);
    add_transition(arena, start, '\0', inner.start);
    add_transition(arena, start, '\0', accept);
    add_transition(arena, inner.accept, '\0', inner.start);
    add_transition(arena, inner.accept, '\0', accept);
    inner.accept->is_accepting = 0;
    accept->is_accepting = 1;
    return (NFA){start, accept};
}