// tmain.c
#define UNITY_BUILD // 启用 Unity Build

#include <stdio.h>
#include <stdlib.h> // For exit, EXIT_FAILURE

// --- Include ALL necessary .h and .c files ---

// Arena
#include "src/arena.h"      // Assuming path structure
#include "src/arena.c"

// NFA (Fragment Version)
#include "src/nfa.h"
#include "src/nfa.c"

// Lexer
#include "src/lexer.h"      // Make sure lexer.h defines Token struct and types
#include "src/lexer.c"

#include "src/matcher.h"      // Make sure lexer.h defines Token struct and types
#include "src/matcher.c"

#include "src/parser.h"      // Make sure lexer.h defines Token struct and types
#include "src/parser.c"

int main() {
    struct Arena* arena = arena_create(1024 * 10);

    const char* regex = "ab*a";//[aba, aa, abba, ....]
    const char* input = "abba";

    State* start = parse_regex(regex, arena);
    int match = simulate_nfa(start, input, arena);

    printf("Regex: \"%s\"\nInput: \"%s\"\nMatch: %s\n",
        regex, input, match ? "YES" : "NO");

    arena_free(arena);
    return 0;
}