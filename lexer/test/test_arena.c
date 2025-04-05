// test/test_arena.c
#include <stdio.h>
#include <stdlib.h>

// Framework header (sibling)
#include "tiny_test_framework.h"
// Module header (relative path to src)
#include "../src/arena.h"

// --- Individual Test Functions ---

static void test_arena_creation(void) {
    struct Arena *a = arena_create(100);
    ASSERT_NOT_NULL(a); // Check if creation succeeded
    // Optional: Check internal state if Arena struct definition is visible
    // ASSERT_EQ_SIZE(100, a->size);
    // ASSERT_EQ_SIZE(0, a->offset);
    arena_free(a);
}

static void test_arena_allocation(void) {
    struct Arena *a = arena_create(50);
    ASSERT_NOT_NULL(a);

    void* p1 = arena_alloc(a, 10);
    ASSERT_NOT_NULL(p1);

    void* p2 = arena_alloc(a, 20);
    ASSERT_NOT_NULL(p2);
    ASSERT_TRUE(p1 != p2); // Pointers should differ

    // Optional: Check contiguity if expected (beware alignment)
    // ASSERT_TRUE((char*)p2 == (char*)p1 + 10);

    // Check offset if Arena struct is visible
    // ASSERT_EQ_SIZE(30, a->offset);

    arena_free(a);
}

static void test_arena_oom(void) {
    struct Arena *a = arena_create(20);
    ASSERT_NOT_NULL(a);

    void* p1 = arena_alloc(a, 15);
    ASSERT_NOT_NULL(p1);

    void* p_oom = arena_alloc(a, 10); // Request more than available (20-15=5)
    ASSERT_NULL(p_oom); // Expect allocation to fail

    // Ensure previous allocation is still valid
    // You could try writing to p1 here if desired (e.g., *((char*)p1) = 'A';)

    arena_free(a);
}

static void test_arena_null_handling(void) {
    ASSERT_NULL(arena_alloc(NULL, 10));
    arena_free(NULL); // Should not crash
}


// --- Test Registration Function ---
// This function is called by main to register all tests in this file.
void register_arena_tests(void) {
    register_test("arena_creation", test_arena_creation);
    register_test("arena_allocation", test_arena_allocation);
    register_test("arena_oom", test_arena_oom);
    register_test("arena_null_handling", test_arena_null_handling);
}