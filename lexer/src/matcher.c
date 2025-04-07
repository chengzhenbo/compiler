#include "matcher.h"
#include <string.h>

typedef struct StateList {
    State* state;
    struct StateList* next;
} StateList;

static void add_state(struct Arena* arena, StateList** list, State* s) {
    for (StateList* p = *list; p; p = p->next)
        if (p->state == s) return;

    StateList* node = (StateList*)arena_alloc(arena, sizeof(StateList));
    node->state = s;
    node->next = *list;
    *list = node;
}

static void epsilon_closure(struct Arena* arena, StateList** list, State* s) {
    add_state(arena, list, s);
    for (Transition* t = s->transitions; t; t = t->next)
        if (t->symbol == '\0')
            epsilon_closure(arena, list, t->target);
}

int simulate_nfa(State* start, const char* input, struct Arena* arena) {
    StateList* current = NULL;
    epsilon_closure(arena, &current, start);

    for (const char* p = input; *p; ++p) {
        StateList* next = NULL;
        for (StateList* node = current; node; node = node->next) {
            for (Transition* t = node->state->transitions; t; t = t->next) {
                if (t->symbol == *p) {
                    epsilon_closure(arena, &next, t->target);
                }
            }
        }
        current = next;
    }

    for (StateList* node = current; node; node = node->next)
        if (node->state->is_accepting)
            return 1;

    return 0;
}