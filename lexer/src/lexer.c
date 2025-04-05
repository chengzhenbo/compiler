// lexer.c
#include <stdio.h>
#include <ctype.h>
#include "lexer.h"

Token* lexer_next_token(const char** regex, 
                        struct Arena* arena) 
{
    if (regex == NULL || *regex == NULL || arena == NULL) {
        return NULL;
    }

    while (isspace(**regex)) {
        (*regex)++;
    }
    Token* token = (Token*)arena_alloc(arena, sizeof(Token));
    if (!token) {
        return NULL;
    }

    char c = **regex;
    if (c == '\0') {
        token->type = T_EOF;
        return token;
    }else if (isalnum(c)) {
        token->type = T_CHAR;
        token->value = c;
        (*regex)++;
    } else if (c == '*') {
        token->type = T_STAR;
        token->value = '*';
        (*regex)++;
    } else{
        token->type = T_INVALID;
        token->value = c;
        (*regex)++; // 消费无效字符以避免无限循环
    }
    return token;
}