// test/test_arena.c
#include <stdio.h>
#include <stdlib.h>

// Framework header (sibling)
#include "tiny_test_framework.h"
// Module header (relative path to src)
#include "../src/arena.h"

#include "../src/lexer.h" // Adjust paths as needed

// --- Helper for Lexer Tests (more robust assertions) ---
static void expect_token(struct Arena* arena, 
                         const char** current_pos,
                         TokenType expected_type, 
                         char expected_value)
{
    Token* token = lexer_next_token(current_pos, arena);
    ASSERT_NOT_NULL(token); // Should always get a token back unless OOM/invalid input

    // Check type first
    ASSERT_EQ_INT(expected_type, token->type); // Use specific int comparison

    // Check value only if type matches and it's relevant
    if (token->type == expected_type) {
         if (expected_type == T_CHAR || expected_type == T_STAR || expected_type == T_INVALID) {
             ASSERT_EQ_CHAR(expected_value, token->value); // Use specific char comparison
         }
         if (expected_type == T_EOF) {
             ASSERT_EQ_CHAR('\0', token->value); // Check EOF value explicitly
        }
    }
}

// --- Individual Test Functions ---

static void test_lexer_empty(void) {
    struct Arena* a = arena_create(64);
    ASSERT_NOT_NULL(a);
    const char* input = "";
    expect_token(a, &input, T_EOF, '\0');
    ASSERT_EQ_CHAR('\0', *input); // Pointer shouldn't advance
    arena_free(a);
}

static void test_lexer_simple_chars(void) {
    struct Arena* a = arena_create(64);
    ASSERT_NOT_NULL(a);
    const char* input = "ab1";
    expect_token(a, &input, T_CHAR, 'a');
    expect_token(a, &input, T_CHAR, 'b');
    expect_token(a, &input, T_CHAR, '1');
    expect_token(a, &input, T_EOF, '\0');
    ASSERT_EQ_CHAR('\0', *input);
    arena_free(a);
}

static void test_lexer_star_and_space(void) {
    struct Arena* a = arena_create(128);
    ASSERT_NOT_NULL(a);
    const char* input = " x * \t y ";
    expect_token(a, &input, T_CHAR, 'x');
    expect_token(a, &input, T_STAR, '*');
    expect_token(a, &input, T_CHAR, 'y');
    expect_token(a, &input, T_EOF, '\0');
    ASSERT_EQ_CHAR('\0', *input);
    arena_free(a);
}

static void test_lexer_invalid_char(void) {
    struct Arena* a = arena_create(64);
    ASSERT_NOT_NULL(a);
    const char* input = "a?c";
    expect_token(a, &input, T_CHAR, 'a');
    // Assuming '?' results in T_INVALID token
    expect_token(a, &input, T_INVALID, '?');
    expect_token(a, &input, T_CHAR, 'c');
    expect_token(a, &input, T_EOF, '\0');
    ASSERT_EQ_CHAR('\0', *input);
    arena_free(a);
}

// --- Test Registration Function ---
void register_lexer_tests(void) {
    register_test("lexer_empty_string", test_lexer_empty);
    register_test("lexer_simple_chars", test_lexer_simple_chars);
    register_test("lexer_star_and_space", test_lexer_star_and_space);
    register_test("lexer_invalid_char", test_lexer_invalid_char);
    // Add more calls to register_test for other lexer test functions
}