#define UNITY_BUILD // 启用 Unity Build
#include <stdio.h>

#include "../src/lexer.h"
#include "../src/lexer.c"

#include "../src/arena.h"
#include "../src/arena.c"

int main(){
    struct Arena* arena = arena_create(64);
    Token* token;
    const char regexp_1[] = "   aa*b&";
    const char* ptr_1 = regexp_1;  // 创建一个指针
    while ((token = lexer_next_token(&ptr_1, arena))->type != T_EOF)
    {
        switch (token->type) {
            case T_CHAR:
                printf("token type=T_CHAR, value=%c\n", token->value);
                break;
            case T_STAR:
                printf("token type=T_STAR, value=%c\n", token->value);
                break;
            case T_INVALID:
                printf("token type=T_INVALID, value=%c\n", token->value);
                break;
            case T_EOF:
                printf("token type=T_EOF\n");
                break;
        }
    }

    return 0;
}