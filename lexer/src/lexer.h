// lexer.h
#ifndef LEXER_H
#define LEXER_H

#include "arena.h"

typedef enum {
    T_CHAR,
    T_STAR,
    T_EOF,
    T_INVALID //表示无效或无法识别的字符
} TokenType;

typedef struct {
    TokenType type;
    char value;
} Token;

Token* lexer_next_token(const char** regex, struct Arena* arena);

#endif // LEXER_H