// testmain.c
#define UNITY_BUILD // 启用 Unity Build

#include <stdio.h>
#include <stdlib.h>

#include "src/arena.h"
#include "src/arena.c"

#include "src/lexer.h"
#include "src/lexer.c"

// --- Test Framework & Tests ---
// Include the framework's implementation
#include "tiny_test_framework.h" // Include framework header first
#include "tiny_test_framework.c" // Include framework implementation
#include "test_arena.c"
#include "test_lexer.c"

int main() {
    printf("Registering tests...\n");
    register_arena_tests();
    register_lexer_tests();
    printf("Test registration complete.\n\n");

    int failures = run_all_tests();

    if (failures == 0) {
        printf("\nApplication can proceed safely.\n");
        // ... your normal application code ...
    } else {
         printf("\nTests failed, application might be unstable.\n");
    }

    // Return non-zero if tests failed, often useful for CI/build scripts
    return failures > 0 ? 1 : 0;
}