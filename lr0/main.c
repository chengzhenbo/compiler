#define UNITY_BUILD // 启用 Unity Build

#include <stdio.h>
#include <stdlib.h> 

// Arena
#include "src/arena.h"      
#include "src/arena.c"

#include "src/grammar.h"
#include "src/grammar.c"

#include "src/viable_prefix_dfa.h"
#include "src/viable_prefix_dfa.c"

int main(){
    Arena* grammar_arena = arena_create(1024*1024);
    Arena* dfa_arena = arena_create(1024 * 1024);
    if (!grammar_arena || !dfa_arena) {
        fprintf(stderr, "Failed to create arena\n");
        if (grammar_arena) arena_free(grammar_arena);
        if (dfa_arena) arena_free(dfa_arena);
        return 1;
    }

    char filename[] = "grammar.txt";
    Grammar* grammar = read_grammar(filename, grammar_arena);
    
    printf("---文法的非终结符---\n");
    for(int i = 0; i < grammar->nonterminals_count; i++){
        printf("Nondermials[%d], %c\n", i, grammar->nonterminals[i]);
    }
    printf("---文法的终结符---\n");
    for(int i = 0; i < grammar->terminals_count; i++){
        printf("Dermials[%d], %c\n", i, grammar->terminals[i]);
    }
    printf("---文法的产生式---\n");
    for (int i = 0; i < grammar->rule_count; i++) {
        printf("Rule[%d]: %c -> %s\n", i, grammar->rules[i].left_hs, grammar->rules[i].right_hs);
    }
    // 构建 DFA
    DFA dfa;
    build_viable_prefix_dfa(grammar, &dfa, dfa_arena);
    print_dfa(&dfa);

    grammar_free(grammar);
    dfa_free(&dfa);
    arena_free(grammar_arena);
    arena_free(dfa_arena);
    return 0;
}