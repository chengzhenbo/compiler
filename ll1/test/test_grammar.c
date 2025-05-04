
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "test_framework.h"

#include "../src/arena.h"      
#include "../src/arena.c"

#include "../src/grammar.h"
#include "../src/grammar.c"





// --- Test functions ---
TEST(test_is_terminal_nonterminal) {
    ASSERT(grammar_is_terminal('a'));
    ASSERT(grammar_is_terminal('+'));
    ASSERT(!grammar_is_terminal('A'));
    ASSERT(grammar_is_nonterminal('S'));
    ASSERT(!grammar_is_nonterminal('a'));
}

TEST(test_add_unique_symbol) {
    char list[10];
    uint8_t count = 0;

    GrammarResultVoid rv_1 = grammar_add_unique_symbol(list, &count, 'a');
    GrammarResultVoid rv_2 = grammar_add_unique_symbol(list, &count, 'b');
    GrammarResultVoid rv_3 = grammar_add_unique_symbol(list, &count, 'a');  // duplicate
    ASSERT(rv_1.status==GRAMMAR_OK);
    ASSERT(rv_2.status==GRAMMAR_OK);
    ASSERT(rv_3.status==GRAMMAR_OK);
    ASSERT(count == 2);
    ASSERT(list[0] == 'a');
    ASSERT(list[1] == 'b');
}

TEST(test_remove_spaces) {
    char input[] = "  A  ->  a B  ";
    line_remove_spaces(input);
    ASSERT_STR_EQ(input, "A->aB");
}

TEST(test_extract_lhs) {
    GrammarResultChar res = grammar_extract_lhs("S->aB");
    ASSERT(res.status == GRAMMAR_OK);
    ASSERT(res.value == 'S');

    res = grammar_extract_lhs("->aB");
    ASSERT(res.status == GRAMMAR_ERROR_INVALID_FORMAT);

    res = grammar_extract_lhs("SS->aB");
    ASSERT(res.status == GRAMMAR_ERROR_INVALID_FORMAT);

    res = grammar_extract_lhs("s->aB");
    ASSERT(res.status == GRAMMAR_ERROR_INVALID_NONTERMINAL);
}

TEST(test_extract_rhs) {
    GrammarResultString res = grammar_extract_rhs("S->aB");
    ASSERT(res.status == GRAMMAR_OK);
    ASSERT_STR_EQ(res.value, "aB");

    res = grammar_extract_rhs("S->   a|b");
    ASSERT(res.status == GRAMMAR_OK);
    ASSERT_STR_EQ(res.value, "a|b");

    res = grammar_extract_rhs("S->");
    ASSERT(res.status == GRAMMAR_ERROR_INVALID_FORMAT);
}

TEST(test_collect_rhs_symbols) {
    Arena* arena = arena_create(1024 * 10);
    GrammarResultGrammar r = init_grammar(arena);
    ASSERT(r.status == GRAMMAR_OK);
    Grammar* g = r.value;

    GrammarResultVoid res = grammar_collect_rhs_symbols("aB#", g);
    ASSERT(res.status == GRAMMAR_OK);

    ASSERT(g->terminals_count == 2);     // a, #
    ASSERT(g->nonterminals_count == 1);  // B

    arena_free(arena);
}

TEST(test_parse_rhs) {
    Arena* arena = arena_create(1024 * 1024);
    GrammarResultGrammar r = init_grammar(arena);
    ASSERT(r.status == GRAMMAR_OK);
    Grammar* g = r.value;

    char rhs_buf[] = "aB|#";
    GrammarResultVoid res = grammar_parse_rhs(g, 'S', rhs_buf, arena);
    ASSERT(res.status == GRAMMAR_OK);

    ASSERT(g->rule_count == 2);
    ASSERT(g->rules[0].left_hs == 'S');
    ASSERT_STR_EQ(g->rules[0].right_hs, "aB");
    ASSERT_STR_EQ(g->rules[1].right_hs, "#");

    arena_free(arena);
}

TEST(test_process_line) {
    Arena* arena = arena_create(1024 * 10);
    GrammarResultGrammar r = init_grammar(arena);
    ASSERT(r.status == GRAMMAR_OK);
    Grammar* g = r.value;

    char line[] = "S -> aB | b";
    GrammarResultVoid res = grammar_process_line(line, g, arena);
    ASSERT(res.status == GRAMMAR_OK);
    ASSERT(g->rule_count == 2);

    arena_free(arena);
}

TEST(test_read_grammar) {
    FILE* f = fopen("temp_grammar.txt", "w");
    fprintf(f, "S -> aB\nB -> b\n");
    fclose(f);

    Arena* arena = arena_create(1024 * 1024);
    GrammarResultGrammar res = read_grammar("temp_grammar.txt", arena);
    ASSERT(res.status == GRAMMAR_OK);
    Grammar* g = res.value;

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