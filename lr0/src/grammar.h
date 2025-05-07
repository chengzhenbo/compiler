#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "arena.h"
#include <stdbool.h>
#include <stdint.h>


#define GRAMMAR_MAX_SYMBOLS 64
#define GRAMMAR_MAX_RULES 128
#define GRAMMAR_MAX_LINE_LEN 256

#ifdef DEBUG_GRAMMAR
#define GRAMMAR_DEBUG(fmt, ...) fprintf(stderr, "[GRAMMAR DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define GRAMMAR_DEBUG(fmt, ...) ((void)0)
#endif

typedef struct Rule
{
    char left_hs;   //产生式左边的非终结符
    char* right_hs; //产生式右边的串，包括终结符与非终结符
    uint8_t right_hs_count;
} Rule;

typedef struct Grammar
{
    Rule* rules;
    uint8_t rule_count;
    char nonterminals[GRAMMAR_MAX_SYMBOLS];
    uint8_t nonterminals_count;
    char terminals[GRAMMAR_MAX_SYMBOLS];
    uint8_t terminals_count;
} Grammar;

// ==== 错误码 ====
typedef enum {
    GRAMMAR_OK = 0,
    GRAMMAR_ERROR_INVALID_ARGUMENT,
    GRAMMAR_ERROR_INVALID_FORMAT,
    GRAMMAR_ERROR_INVALID_NONTERMINAL,
    GRAMMAR_ERROR_ALLOCATION_FAILED,
    GRAMMAR_ERROR_IO_FAILED,
    GRAMMAR_ERROR_TOO_MANY_RULES,
    GRAMMAR_ERROR_TOO_MANY_SYMBOLS
} GrammarStatus;

/// 用于返回 Grammar*
typedef struct GrammarResultGrammar{
    GrammarStatus status;
    Grammar* value;
} GrammarResultGrammar;

/// 用于返回 char
typedef struct GrammarResultChar{
    GrammarStatus status;
    char value;
} GrammarResultChar;

/// 用于返回字符串
typedef struct GrammarResultString{
    GrammarStatus status;
    char* value;
} GrammarResultString;

/// 用于无值返回
typedef struct GrammarResultVoid{
    GrammarStatus status;
} GrammarResultVoid;

bool grammar_is_terminal(char c);
bool grammar_is_nonterminal(char c);

GrammarResultGrammar read_grammar(const char* filename, Arena* arena);
void print_grammar(const Grammar* grammar);
void grammar_free(Grammar* grammar);
#endif