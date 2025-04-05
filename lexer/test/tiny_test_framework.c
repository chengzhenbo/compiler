#include <stdio.h>
#include <stdlib.h>
#include "tiny_test_framework.h"

#define MAX_TESTS 100 // Adjust as needed

typedef struct {
    const char* name;
    TestFunc func;
} TestCaseInternal;

static TestCaseInternal g_test_registry[MAX_TESTS];
static int g_test_count = 0;

// Track overall failures
static int g_total_failed = 0;
// Track failures within the currently executing test function
int g_current_test_failed = 0; // Definition for the extern variable


void register_test(const char* test_name, TestFunc func) {
    if (g_test_count < MAX_TESTS) {
        g_test_registry[g_test_count].name = test_name;
        g_test_registry[g_test_count].func = func;
        g_test_count++;
    } else {
        fprintf(stderr, "ERROR: Test registry full. Increase MAX_TESTS.\n");
        exit(1); // Cannot proceed
    }
}

// Internal helper called by assertion macros on failure
void _test_fail(const char* expression, const char* file, int line, const char* message) {
    fprintf(stderr, "  [FAIL] %s:%d: Assertion '%s' failed", file, line, expression);
    if (message) {
        fprintf(stderr, " - %s", message);
    }
    fprintf(stderr, "\n");
}


int run_all_tests(void) {
    printf("=== Running Tests ===\n");
    g_total_failed = 0;
    int tests_passed = 0;

    for (int i = 0; i < g_test_count; ++i) {
        printf("Running test: %s ...\n", g_test_registry[i].name);
        g_current_test_failed = 0; // Reset failure flag for this test

        // --- Optional Setup ---
        // setup(); // Call a global setup function if defined

        // --- Execute Test ---
        g_test_registry[i].func();

        // --- Optional Teardown ---
        // teardown(); // Call a global teardown function if defined

        // --- Report Result ---
        if (g_current_test_failed == 0) {
            printf("  [PASS]\n");
            tests_passed++;
        } else {
            // Failure message already printed by assertion macro
            g_total_failed += g_current_test_failed; // Accumulate failures
        }
        printf("\n");
    }

    printf("=== Test Summary ===\n");
    printf("Total tests run: %d\n", g_test_count);
    printf("Tests passed:    %d\n", tests_passed);
    printf("Tests failed:    %d\n", g_test_count - tests_passed); // Number of tests with at least one failure
    printf("Total assertions failed: %d\n", g_total_failed);
    printf("====================\n");

    return (g_total_failed > 0); // Return 0 if all passed, non-zero otherwise
}