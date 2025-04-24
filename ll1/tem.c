// arena.h
#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

struct Arena;

struct Arena* arena_create(size_t size);
void* arena_alloc(struct Arena* arena, size_t size);
void arena_free(struct Arena* arena);

#endif // ARENA_H

// arena.c
#include "arena.h"
#include <stdlib.h>

struct Arena {
    char* buffer;
    size_t size;
    size_t offset;
};

struct Arena* arena_create(size_t size) {
    struct Arena* arena = malloc(sizeof(struct Arena));
    arena->buffer = malloc(size);
    arena->size = size;
    arena->offset = 0;
    return arena;
}

void* arena_alloc(struct Arena* arena, size_t size) {
    if (arena->offset + size > arena->size) return NULL;
    void* ptr = arena->buffer + arena->offset;
    arena->offset += size;
    return ptr;
}

void arena_free(struct Arena* arena) {
    if (arena) {
        free(arena->buffer);
        free(arena);
    }
}

// grammar.h
#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "arena.h"
#include <stdbool.h>

#define MAX_SYMBOLS 64
#define MAX_RULES 128

typedef struct {
    char lhs;
    char* rhs;
} Rule;

typedef struct {
    Rule* rules;
    int rule_count;
    char nonterminals[MAX_SYMBOLS];
    int nonterminal_count;
    char terminals[MAX_SYMBOLS];
    int terminal_count;
} Grammar;

Grammar* read_grammar(const char* filename, struct Arena* arena);

#endif // GRAMMAR_H

// grammar.c
#include <stdio.h>
#include <string.h>
#include <ctype.h>   // isspace
#include <stdbool.h>

#include "grammar.h" // 假设 grammar.h 包含结构体定义
#include "arena.h"

bool is_nonterminal(char c) {
    return (c >= 'A' && c <= 'Z');
}

void add_unique(char* list, int* count, char c) {
    for (int i = 0; i < *count; ++i) {
        if (list[i] == c) return;
    }
    list[(*count)++] = c;
}

Grammar* read_grammar(const char* filename, struct Arena* arena) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open grammar file: %s\n", filename);
        return NULL;
    }

    Grammar* grammar = arena_alloc(arena, sizeof(Grammar));
    if (!grammar) {
        fclose(file);
        return NULL;
    }

    grammar->rules = arena_alloc(arena, MAX_RULES * sizeof(Rule));
    grammar->rule_count = 0;
    grammar->nonterminal_count = 0;
    grammar->terminal_count = 0;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // 忽略空行或注释
        if (line[0] == '\n' || line[0] == '#') continue;

        char* arrow = strstr(line, "->");
        if (!arrow || arrow == line) continue;

        *arrow = '\0';
        char lhs = line[0];
        char* rhs = arrow + 2;

        add_unique(grammar->nonterminals, &grammar->nonterminal_count, lhs);

        char* token = strtok(rhs, "|\n");
        while (token) {
            // 去除开头空格
            while (isspace(*token)) token++;

            Rule* rule = &grammar->rules[grammar->rule_count++];
            rule->lhs = lhs;
            rule->rhs = arena_alloc(arena, strlen(token) + 1);
            strcpy(rule->rhs, token);

            for (char* p = token; *p; ++p) {
                if (isspace(*p)) continue;
                if (*p == 'ε') {
                    add_unique(grammar->terminals, &grammar->terminal_count, 'ε');
                } else if (is_nonterminal(*p)) {
                    add_unique(grammar->nonterminals, &grammar->nonterminal_count, *p);
                } else {
                    add_unique(grammar->terminals, &grammar->terminal_count, *p);
                }
            }

            token = strtok(NULL, "|\n");
        }
    }

    fclose(file);
    return grammar;
}

// first_follow.h
#ifndef FIRST_FOLLOW_H
#define FIRST_FOLLOW_H

#include "grammar.h"
#include "arena.h"

typedef struct {
    char symbol;
    char* first;
    char* follow;
} SymbolSet;

void compute_first_follow(Grammar* grammar, SymbolSet* sets, int* set_count, struct Arena* arena);

#endif // FIRST_FOLLOW_H

// first_follow.c
#include "first_follow.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

static SymbolSet* get_or_create_set(SymbolSet* sets, int* count, char symbol, struct Arena* arena) {
    for (int i = 0; i < *count; i++) {
        if (sets[i].symbol == symbol) return &sets[i];
    }
    SymbolSet* set = &sets[(*count)++];
    set->symbol = symbol;
    set->first = arena_alloc(arena, MAX_SYMBOLS);
    set->follow = arena_alloc(arena, MAX_SYMBOLS);
    set->first[0] = '\0';
    set->follow[0] = '\0';
    return set;
}

static bool add_char(char* set, char c) {
    if (strchr(set, c)) return false;
    int len = strlen(set);
    set[len] = c;
    set[len + 1] = '\0';
    return true;
}

void compute_first_follow(Grammar* grammar, SymbolSet* sets, int* set_count, struct Arena* arena) {
    // First Sets
    bool changed;
    do {
        changed = false;
        for (int i = 0; i < grammar->rule_count; i++) {
            Rule* rule = &grammar->rules[i];
            SymbolSet* lhs_set = get_or_create_set(sets, set_count, rule->lhs, arena);

            bool epsilon_in_rhs = true;
            for (int j = 0; rule->rhs[j]; j++) {
                char sym = rule->rhs[j];
                if (sym == ' ') continue;
                if (is_terminal(sym) || sym == 'ε') {
                    if (add_char(lhs_set->first, sym)) changed = true;
                    epsilon_in_rhs = (sym == 'ε');
                    break;
                } else {
                    SymbolSet* sym_set = get_or_create_set(sets, set_count, sym, arena);
                    for (int k = 0; sym_set->first[k]; k++) {
                        if (sym_set->first[k] != 'ε')
                            if (add_char(lhs_set->first, sym_set->first[k])) changed = true;
                    }
                    if (!strchr(sym_set->first, 'ε')) {
                        epsilon_in_rhs = false;
                        break;
                    }
                }
            }
            if (epsilon_in_rhs) {
                if (add_char(lhs_set->first, 'ε')) changed = true;
            }
        }
    } while (changed);

    // Follow Sets
    SymbolSet* start = get_or_create_set(sets, set_count, 'S', arena);
    add_char(start->follow, '$');

    do {
        changed = false;
        for (int i = 0; i < grammar->rule_count; i++) {
            Rule* rule = &grammar->rules[i];
            int len = strlen(rule->rhs);
            for (int j = 0; j < len; j++) {
                char B = rule->rhs[j];
                if (!is_nonterminal(B)) continue;
                SymbolSet* B_set = get_or_create_set(sets, set_count, B, arena);

                bool epsilon_chain = true;
                for (int k = j + 1; k < len; k++) {
                    char next = rule->rhs[k];
                    if (is_terminal(next)) {
                        if (add_char(B_set->follow, next)) changed = true;
                        epsilon_chain = false;
                        break;
                    }
                    SymbolSet* next_set = get_or_create_set(sets, set_count, next, arena);
                    for (int m = 0; next_set->first[m]; m++) {
                        if (next_set->first[m] != 'ε')
                            if (add_char(B_set->follow, next_set->first[m])) changed = true;
                    }
                    if (!strchr(next_set->first, 'ε')) {
                        epsilon_chain = false;
                        break;
                    }
                }
                if (epsilon_chain) {
                    SymbolSet* A_set = get_or_create_set(sets, set_count, rule->lhs, arena);
                    for (int m = 0; A_set->follow[m]; m++) {
                        if (add_char(B_set->follow, A_set->follow[m])) changed = true;
                    }
                }
            }
        }
    } while (changed);
}

// main.c
#include "grammar.h"
#include "first_follow.h"
#include "arena.h"
#include <stdio.h>

int main() {
    struct Arena* arena = arena_create(10240);
    Grammar* grammar = read_grammar("grammar.txt", arena);
    if (!grammar) {
        printf("Error reading grammar.\n");
        return 1;
    }

    SymbolSet* sets = arena_alloc(arena, MAX_SYMBOLS * sizeof(SymbolSet));
    int set_count = 0;
    compute_first_follow(grammar, sets, &set_count, arena);

    for (int i = 0; i < set_count; i++) {
        printf("%c: FIRST = {%s}, FOLLOW = {%s}\n", sets[i].symbol, sets[i].first, sets[i].follow);
    }

    arena_free(arena);
    return 0;
}
