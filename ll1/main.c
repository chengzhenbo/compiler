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
        printf("%c\n", grammar->nonderminals[i]);
    }
    
    printf("=== Grammar Rules ===\n");
    for(int i = 0; i < grammar->rule_count; i++){
        Rule* r = &grammar->rules[i];
        printf("%c -> %s\n", r->left_hs, r->right_hs);
    }

    arena_free(arena);
    return 0;
}