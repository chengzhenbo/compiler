// test.c
#include <stdio.h>
#include <stdlib.h>

#include "test.h"

#include "arena.h"
#include "lexer.h"

// NFA 实现
#include "nfa.h"


// 定义断言宏
#define ASSERT(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAILED: %s (line %d): %s\n", __FILE__, __LINE__, message); \
        exit(1); \
    } else { \
        printf("PASSED: %s\n", message); \
    } \
} while (0)


// 测试 lexer_next_token 解析 "a * b"
void test_lexer(const char* input) {
    const char* ptr = input;
    struct Arena* arena = arena_create(sizeof(Token) * 100);
    ASSERT(arena != NULL, "Arena should be created successfully for lexer test");

    printf("Parsing test: '%s'\n", input);
    Token* token;
    int char_count = 0;
    while ((token = lexer_next_token(&ptr, arena)) != NULL) {
        if (token->type == T_EOF) {
            printf("EOF\n");
            ASSERT(*ptr == '\0', "Pointer should reach end of input at EOF");
            break;
        } else if (token->type == T_CHAR) {
            printf("CHAR: %c\n", token->value);
            char_count++;
            ASSERT(token->value == 'a' || token->value == 'b', "CHAR token should be 'a' or 'b'");
        } else if (token->type == T_STAR) {
            printf("STAR: %c\n", token->value);
            ASSERT(token->value == '*', "STAR token should be '*'");
        }
    }
    ASSERT(char_count == 2, "Should have exactly 2 CHAR tokens ('a' and 'b')");
    arena_free(arena);
}

// 检查状态是否可达接受状态
static int is_accepting_reachable(State* state, char* visited) {
    if (!state) return 0;
    if (state->is_accepting) return 1;
    if (visited[(size_t)state]) return 0;  // 防止循环
    visited[(size_t)state] = 1;

    for (Transition* t = state->transitions; t; t = t->next) {
        if (t->symbol == '\0' && is_accepting_reachable(t->target, visited)) {
            return 1;
        }
    }
    return 0;
}

// NFA 匹配函数
int match_nfa(State* start, const char* input) {
    if (!start) return 0;
    State* current = start;
    char* visited = calloc(1000, sizeof(char));  // 动态分配 visited 数组
    if (!visited) {
        fprintf(stderr, "Failed to allocate visited array\n");
        exit(1);
    }

    for (int i = 0; ; i++) {
        if (input[i] == '\0') {
            int result = is_accepting_reachable(current, visited);
            free(visited);
            return result;
        }

        int matched = 0;
        for (Transition* t = current->transitions; t; t = t->next) {
            if (t->symbol == (unsigned char)input[i]) {
                current = t->target;
                matched = 1;
                break;
            }
        }
        if (!matched) {
            for (Transition* t = current->transitions; t; t = t->next) {
                if (t->symbol == '\0') {
                    current = t->target;
                    i--;  // 回退一步，重新检查当前字符
                    matched = 1;
                    break;
                }
            }
            if (!matched) {
                free(visited);
                return 0;
            }
        }
    }
}

void test_char_nfa(struct Arena* arena) {
    State* nfa = create_char_nfa('a', arena);
    ASSERT(match_nfa(nfa, "a") == 1, "NFA should match 'a'");
    ASSERT(match_nfa(nfa, "b") == 0, "NFA should not match 'b'");
}

void test_star_nfa(struct Arena* arena) {
    State* char_nfa = create_char_nfa('a', arena);
    State* star_nfa = create_star_nfa(char_nfa, arena);

    ASSERT(match_nfa(star_nfa, "") == 1, "Star NFA should match empty string");
    ASSERT(match_nfa(star_nfa, "a") == 1, "Star NFA should match 'a'");
    ASSERT(match_nfa(star_nfa, "aa") == 1, "Star NFA should match 'aa'");
    ASSERT(match_nfa(star_nfa, "b") == 0, "Star NFA should not match 'b'");
}

// 运行所有测试
void run_all_tests(const char* input) {
    printf("Running all tests...\n");
    test_lexer(input);

    // printf("Running NFA tests with Arena...\n");
    // struct Arena* arena = arena_create(sizeof(State) * 100 + sizeof(Transition) * 100);
    // ASSERT(arena != NULL, "Arena should be created successfully");

    // test_char_nfa(arena);
    // test_star_nfa(arena);

    // arena_free(arena);
    printf("All tests passed!\n");
}