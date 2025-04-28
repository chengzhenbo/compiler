#include "viable_prefix_dfa.h"
#include "grammar.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

// 打印 DFA
void print_dfa(const DFA* dfa) {
    printf("=== DFA States ===\n");
    for (uint8_t i = 0; i < dfa->state_count; i++) {
        printf("State %d:\n", i);
        for (uint8_t j = 0; j < dfa->states[i].item_count; j++) {
            const DFAItem* item = &dfa->states[i].items[j];
            printf("  %c -> ", item->left_symbol);
            for (uint8_t k = 0; k < item->right_len; k++) {
                if (k == item->dot) printf(".");
                printf("%c", item->right_symbols[k]);
            }
            if (item->dot == item->right_len) printf(".");
            printf("\n");
        }
    }

    printf("\n=== DFA Transitions ===\n");
    for (uint8_t i = 0; i < dfa->transition_count; i++) {
        printf("  %d --%c--> %d\n", dfa->transitions[i].from_state,
               dfa->transitions[i].symbol, dfa->transitions[i].to_state);
    }
}

// 判断两个 DFA 项是否相等
static int item_equal(const DFAItem* a, const DFAItem* b) {
    return (a->left_symbol == b->left_symbol) &&
           (a->right_len == b->right_len) &&
           (a->dot == b->dot) &&
           (strncmp(a->right_symbols, b->right_symbols, a->right_len) == 0);
}

// 判断两个 ItemSet 是否相等
static bool itemset_equal(const ItemSet* a, const ItemSet* b) {
    if (a->item_count != b->item_count) return false;
    for (uint8_t i = 0; i < a->item_count; i++) {
        int found = 0;
        for (uint8_t j = 0; j < b->item_count; j++) {
            if (item_equal(&a->items[i], &b->items[j])) {
                found = 1;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

// 查找 DFA 中是否已存在某个项集
static int find_state(const DFA* dfa, const ItemSet* set) {
    for (uint8_t i = 0; i < dfa->state_count; i++) {
        if (itemset_equal(&dfa->states[i], set)) {
            return i;
        }
    }
    return -1;
}

static void closure(ItemSet* set, const Grammar* grammar, Arena* arena){
    uint8_t i = 0;
    while(i < set->item_count){
        DFAItem* item = &set->items[i++];
        if(item->dot >= item->right_len) continue;

        char next_symbol = item->right_symbols[item->dot];
        if(isupper(next_symbol)){
            for(int j = 0; j < grammar->rule_count; j++){
                if(grammar->rules[j].left_hs == next_symbol){
                    DFAItem new_item = {
                        .left_symbol = next_symbol,
                        .right_symbols = grammar->rules[j].right_hs,
                        .right_len = strlen(grammar->rules[j].right_hs),
                        .dot = 0
                    };
                    int exists = 0;
                    for(uint8_t k = 0; k < set->item_count; k++){
                        if(item_equal(&set->items[k], &new_item)){
                            exists = 1;
                            break;
                        }
                    }
                    if (!exists) {
                        if (set->item_count >= set->item_capacity) {
                            size_t new_capacity = set->item_capacity ? set->item_capacity * 2 : 8;
                            DFAItem* new_items = arena_alloc(arena, new_capacity * sizeof(DFAItem));
                            if(!new_items){
                                fprintf(stderr, "Invalid newitems.\n");
                                exit(EXIT_FAILURE);
                            }
                            memcpy(new_items, set->items, set->item_count * sizeof(DFAItem));
                            set->items = new_items;
                            set->item_capacity = new_capacity;
                        }
                        set->items[set->item_count++] = new_item;
                    }
                }
            }
        }
    }
}

// 计算项集在给定符号下的转移
static void goto_set(const ItemSet* set, char symbol, const Grammar* grammar, ItemSet* out, Arena* arena) {
    out->item_count = 0;
    for (uint8_t i = 0; i < set->item_count; i++) {
        const DFAItem* item = &set->items[i];
        if (item->dot < item->right_len && item->right_symbols[item->dot] == symbol) {
            if (out->item_count >= out->item_capacity) {
                size_t new_capacity = out->item_capacity ? out->item_capacity * 2 : 8;
                DFAItem* new_items = arena_alloc(arena, new_capacity * sizeof(DFAItem));
                memcpy(new_items, out->items, out->item_count * sizeof(DFAItem));
                out->items = new_items;
                out->item_capacity = new_capacity;
            }
            out->items[out->item_count++] = (DFAItem){
                .left_symbol = item->left_symbol,
                .right_symbols = item->right_symbols,
                .right_len = item->right_len,
                .dot = item->dot + 1
            };
        }
    }
    closure(out, grammar, arena);
}

void build_viable_prefix_dfa(const Grammar* grammar, DFA* dfa, Arena* arena){
    if(!grammar || grammar->rule_count <= 0){
        fprintf(stderr, "Invalid or empty grammar.\n");
        exit(EXIT_FAILURE);
    }
    memset(dfa, 0, sizeof(DFA));
    dfa->arena = arena;

    ItemSet start = {0};
    start.item_capacity = 8;
    start.items = arena_alloc(arena, start.item_capacity*sizeof(DFAItem));
    if(!start.items){
        fprintf(stderr, "Invalid start dfa item.\n");
        exit(EXIT_FAILURE);
    }
    start.items[0] = (DFAItem){
        .left_symbol = grammar->rules[0].left_hs,
        .right_symbols = grammar->rules[0].right_hs,
        .right_len = strlen(grammar->rules[0].right_hs),
        .dot = 0
    };
    start.item_count = 1;
    closure(&start, grammar, arena);

    dfa->state_capacity = 16;
    dfa->states = arena_alloc(arena, dfa->state_capacity*sizeof(ItemSet));
    if(!dfa->states){
        fprintf(stderr, "Invalid dfa state.\n");
        exit(EXIT_FAILURE);
    }
    dfa->states[dfa->state_count++] = start;

    dfa->transition_capacity = 32;
    dfa->transitions = arena_alloc(arena, dfa->transition_capacity * sizeof(Transition));
    if(!dfa->transitions){
        fprintf(stderr, "Invalid dfa transitions.\n");
        exit(EXIT_FAILURE);
    }
    for (uint8_t i = 0; i < dfa->state_count; i++) {
        const ItemSet* current = &dfa->states[i];
        char seen_symbols[256] = {0};

        for (uint8_t j = 0; j < current->item_count; j++) {
            char sym = current->items[j].right_symbols[current->items[j].dot];
            if (sym && !seen_symbols[(unsigned char)sym]) {
                seen_symbols[(unsigned char)sym] = 1;

                ItemSet next = {0};
                next.item_capacity = 8;
                next.items = arena_alloc(arena, next.item_capacity * sizeof(DFAItem));
                goto_set(current, sym, grammar, &next, arena);

                int existing = find_state(dfa, &next);
                if (existing == -1) {
                    if (dfa->state_count >= dfa->state_capacity) {
                        size_t new_capacity = dfa->state_capacity * 2;
                        ItemSet* new_states = arena_alloc(arena, new_capacity * sizeof(ItemSet));
                        memcpy(new_states, dfa->states, dfa->state_count * sizeof(ItemSet));
                        dfa->states = new_states;
                        dfa->state_capacity = new_capacity;
                    }
                    dfa->states[dfa->state_count++] = next;
                    existing = dfa->state_count - 1;
                } else {
                    next.items = NULL;
                }

                if (dfa->transition_count >= dfa->transition_capacity) {
                    size_t new_capacity = dfa->transition_capacity * 2;
                    Transition* new_transitions = arena_alloc(arena, new_capacity * sizeof(Transition));
                    memcpy(new_transitions, dfa->transitions, dfa->transition_count * sizeof(Transition));
                    dfa->transitions = new_transitions;
                    dfa->transition_capacity = new_capacity;
                }
                dfa->transitions[dfa->transition_count++] = (Transition){
                    .from_state = i,
                    .symbol = sym,
                    .to_state = existing
                };
            }
        }
    }

}

void dfa_free(DFA* dfa){
    if (dfa && dfa->arena) {
        memset(dfa, 0, sizeof(DFA));
    }
}