
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../src/arena.h"      
#include "../src/arena.c"

#include "../src/grammar.h"
#include "../src/grammar.c"


// Very small test helpers
int failed = 0;
#define TEST(name) void name()
#define RUN_TEST(name) printf("\n\033[1m%s\n\033[0m", #name); name()
#define ASSERT(expr) if (!(expr)) { \
  failed = 1; \
  printf("\033[0;31mFAIL: %s\n\033[0m", #expr); \
} else { \
  printf("\033[0;32mPASS: %s\n\033[0m", #expr); \
}
#define ASSERT_STR_EQ(str1, str2) if (!(strcmp(str1, str2) == 0)) { \
  failed = 1; \
  printf("\033[0;31mFAIL: %s != %s\n\033[0m", str1, str2); \
} else { \
  printf("\033[0;32mPASS: %s == %s\n\033[0m", str1, str2); \
}


// --- Test functions ---
TEST(test_is_terminal_nonterminal) {
    ASSERT(grammar_is_terminal('a'));
    ASSERT(grammar_is_terminal('+'));
    ASSERT(!grammar_is_terminal('A'));
    ASSERT(grammar_is_nonterminal('S'));
    ASSERT(!grammar_is_nonterminal('A'));
}

TEST(test_add_unique_symbol) {
    char list[10];
    uint8_t count = 0;

    grammar_add_unique_symbol(list, &count, 'a');
    grammar_add_unique_symbol(list, &count, 'b');
    grammar_add_unique_symbol(list, &count, 'a');  // duplicate

    ASSERT(count == 2);
    ASSERT(list[0] == 'a');
    ASSERT(list[1] == 'b');
}

TEST(test_remove_spaces) {
    char input[] = "  A  ->  a B  ";
    remove_spaces(input);
    ASSERT_STR_EQ(input, "A->aB");
}

TEST(test_extract_lhs) {
    char lhs;
    ASSERT(grammar_extract_lhs("S->aB", &lhs) == GRAMMAR_OK);
    ASSERT(lhs == 'S');

    ASSERT(grammar_extract_lhs("->aB", &lhs) == GRAMMAR_ERROR_INVALID_FORMAT);
    ASSERT(grammar_extract_lhs("SS->aB", &lhs) == GRAMMAR_ERROR_INVALID_FORMAT);
    ASSERT(grammar_extract_lhs("s->aB", &lhs) == GRAMMAR_ERROR_INVALID_NONTERMINAL);
}

TEST(test_extract_rhs) {
    char* rhs = NULL;
    ASSERT(grammar_extract_rhs("S->aB", &rhs) == GRAMMAR_OK);
    ASSERT_STR_EQ(rhs, "aB");

    ASSERT(grammar_extract_rhs("S->   a|b", &rhs) == GRAMMAR_OK);
    ASSERT_STR_EQ(rhs, "a|b");

    ASSERT(grammar_extract_rhs("S->", &rhs) == GRAMMAR_ERROR_INVALID_FORMAT);
}

TEST(test_collect_rhs_symbols) {
    Arena* arena = arena_create(1024*10);
    Grammar* g = init_grammar(arena);
    grammar_collect_rhs_symbols("aB#", g);

    ASSERT(g->terminals_count == 2);  // a, #
    ASSERT(g->nonterminals_count == 1);  // B

    arena_free(arena);
}

TEST(test_parse_rhs) {
    Arena* arena = arena_create(1024*1024);
    Grammar* g = init_grammar(arena);
    ASSERT(g != NULL);
    char rhs_buf[] = "aB|#"; 
    ASSERT(grammar_parse_rhs(g, 'S', rhs_buf, arena));
    ASSERT(g->rule_count == 2);
    ASSERT(g->rules[0].left_hs == 'S');
    ASSERT_STR_EQ(g->rules[0].right_hs, "aB");
    ASSERT_STR_EQ(g->rules[1].right_hs, "#");

    arena_free(arena);
}

TEST(test_process_line) {
    Arena* arena = arena_create(1024*10);
    Grammar* g = init_grammar(arena);

    char line[] = "S -> aB | b";
    ASSERT(grammar_process_line(line, g, arena));
    ASSERT(g->rule_count == 2);

    arena_free(arena);
}

TEST(test_read_grammar) {
    FILE* f = fopen("temp_grammar.txt", "w");
    fprintf(f, "S -> aB\nB -> b\n");
    fclose(f);

    Arena* arena = arena_create(1024*1024);
    Grammar* g = read_grammar("temp_grammar.txt", arena);
    ASSERT(g != NULL);
    ASSERT(g->rule_count == 2);
    ASSERT(g->rules[0].left_hs == 'S');
    ASSERT_STR_EQ(g->rules[0].right_hs, "aB");
    ASSERT_STR_EQ(g->rules[1].right_hs, "b");

    remove("temp_grammar.txt");
    arena_free(arena);
}

// --- Main ---
int main() {
    RUN_TEST(test_is_terminal_nonterminal);
    RUN_TEST(test_add_unique_symbol);
    RUN_TEST(test_remove_spaces);
    RUN_TEST(test_extract_lhs);
    RUN_TEST(test_extract_rhs);
    RUN_TEST(test_collect_rhs_symbols);
    RUN_TEST(test_parse_rhs);
    RUN_TEST(test_process_line);
    RUN_TEST(test_read_grammar);

    return failed;
}