// nfa.h
#ifndef NFA_H
#define NFA_H

#include "arena.h"

typedef struct Transition {
    unsigned char symbol;      // 转移字符（'\0' 表示 epsilon）
    struct State* target;      // 目标状态
    struct Transition* next;   // 下一条转移
} Transition;

typedef struct State {
    Transition* transitions;   // 转移链表
    int is_accepting;          // 是否为接受状态
} State;

State* create_state(struct Arena* arena);
State* create_char_nfa(char c, struct Arena* arena);
State* create_star_nfa(State* sub_nfa_start, struct Arena* arena);

#endif // NFA_H