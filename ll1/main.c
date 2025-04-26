#define UNITY_BUILD // 启用 Unity Build

#include <stdio.h>
#include <stdlib.h> 

// Arena
#include "src/arena.h"      
#include "src/arena.c"

#include "src/grammar_utils.h"
#include "src/grammar_utils.c"

#include "src/grammar.h"
#include "src/grammar.c"

#include "src/first_follow.h"
#include "src/first_follow.c"

#include "src/first_set.h"
#include "src/first_set.c"

#include "src/follow_set.h"
#include "src/follow_set.c"

int main(){
    struct Arena* arena = arena_create(1024*10);
    char filename[] = "grammar.txt";

    Grammar* grammar = read_grammar(filename, arena);
    printf("---文法的非终结符---\n");
    for(int i = 0; i < grammar->nontermials_count; i++){
        printf("Nondermials[%d], %c\n", i, grammar->nonterminals[i]);
    }
    printf("---文法的终结符---\n");
    for(int i = 0; i < grammar->termials_count; i++){
        printf("Dermials[%d], %c\n", i, grammar->terminals[i]);
    }
    printf("---文法的产生式---\n");
    for (int i = 0; i < grammar->rule_count; i++) {
        printf("Rule[%d]: %c -> %s\n", i, grammar->rules[i].left_hs, grammar->rules[i].right_hs);
    }

    SymbolSet* sets = arena_alloc(arena, MAX_SYMBOLS * sizeof(SymbolSet));
    int set_count = 0;
    printf("---非终结符的First集---\n");
    compute_first_sets(grammar, sets, &set_count, arena);
    for(int i = 0; i < set_count; i++){
        printf("First set[%c] : %s\n", sets[i].symbol, sets[i].first);
    }
    printf("---非终结符的Follow集---\n");
    compute_follow_sets(grammar, sets, &set_count, arena);
    for(int i = 0; i < set_count; i++){
        printf("Follow set[%c] : %s\n", sets[i].symbol, sets[i].follow);
    }

    arena_free(arena);
    return 0;
}