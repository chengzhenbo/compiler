#define UNITY_BUILD // 启用 Unity Build

#include <stdio.h>
#include <stdlib.h> // For exit, EXIT_FAILURE

// Arena
#include "src/arena.h"      // Assuming path structure
#include "src/arena.c"

#include "src/grammar.h"
#include "src/grammar.c"

int main(){
    struct Arena* arena = arena_create(1024*10);
    char filename[] = "grammar.txt";

    Grammar* grammar = read_grammar(filename, arena);

    for(int i = 0; i < grammar->nondermials_count; i++){
        printf("nondermials[%d], %c\n", i, grammar->nonderminals[i]);
    }
    for(int i = 0; i < grammar->dermials_count; i++){
        printf("dermials[%d], %c\n", i, grammar->derminals[i]);
    }

    for (int i = 0; i < grammar->rule_count; i++) {
        printf("rule[%d]: %c -> %s\n", i, grammar->rules[i].left_hs, grammar->rules[i].right_hs);
    }
    

    arena_free(arena);
    return 0;
}