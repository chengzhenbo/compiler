#include "grammar.h"
#include <stdio.h>
#include <string.h>

Grammar* read_grammar(const char* filename, Arena* arena){
    FILE* file = fopen(filename, "r");
    if (!file){
        fprintf(stderr, "Error: Failed to open the file.\n");
        return NULL;
    }
    Grammar* grammar = arena_alloc(arena, sizeof(Grammar));
    if(!grammar){
        fprintf(stderr, "Error: Failed to allocate memory for Grammar struct.\n");
        return NULL; // 内存分配失败
    }
    Rule* rules = arena_alloc(arena, MAX_RULES*sizeof(Rule));
    if(!rules){
        fprintf(stderr, "Error: Failed to allocate memory for rule struct.\n");
        return NULL;
    }
    grammar->rules = rules;
    grammar->rule_count = 1;
    strcpy(grammar->nonderminals, "AB");
    grammar->nondermials_count = 2;
    strcpy(grammar->derminals, "ab");
    grammar->dermials_count = 2;

    rules[0].left_hs = 'A';
    rules[0].right_hs = arena_alloc(arena, 3*sizeof(char));
    strcpy(rules[0].right_hs, "aB");

    return grammar;
}