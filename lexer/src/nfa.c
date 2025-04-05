// nfa.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nfa.h"

// 在 Arena 中创建一个新的 NFA 状态
State* create_state(struct Arena* arena) {
    State* state = (State*)arena_alloc(arena, sizeof(State));
    if (!state) {
        fprintf(stderr, "Failed to allocate State from Arena\n");
        exit(1);
    }
    state->transitions = NULL;
    state->is_accepting = 0;
    return state;
}

// 添加转移到状态
static void add_transition(struct Arena* arena, State* from, unsigned char symbol, State* to) {
    Transition* t = (Transition*)arena_alloc(arena, sizeof(Transition));
    if (!t) {
        fprintf(stderr, "Failed to allocate Transition from Arena\n");
        exit(1);
    }
    t->symbol = symbol;
    t->target = to;
    t->next = from->transitions;
    from->transitions = t;
}

// 构建匹配单个字符的 NFA
State* create_char_nfa(char c, struct Arena* arena) {
    State* start = create_state(arena);
    State* accept = create_state(arena);

    add_transition(arena, start, (unsigned char)c, accept);
    accept->is_accepting = 1;

    return start;
}

// 构建支持 '*' 的 NFA（零次或多次重复）
State* create_star_nfa(State* sub_nfa_start, struct Arena* arena) {
    State* start = create_state(arena);
    State* accept = create_state(arena);

    const unsigned char epsilon = '\0';

    // start -> sub_nfa_start (进入子 NFA)
    add_transition(arena, start, epsilon, sub_nfa_start);

    // sub_nfa_start -> accept (跳过子 NFA)
    add_transition(arena, sub_nfa_start, epsilon, accept);

    // sub_nfa_start -> start (循环回去)
    add_transition(arena, sub_nfa_start, epsilon, start);

    // start -> accept (匹配零次的情况)
    add_transition(arena, start, epsilon, accept);

    accept->is_accepting = 1;

    return start;
}