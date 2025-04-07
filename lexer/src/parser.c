#include "parser.h"
#include "lexer.h"

State* parse_regex(const char* regex, struct Arena* arena) {
    State* start = create_state(arena);
    State* current = start;

    const char* p = regex;
    Token* tok;

    while ((tok = lexer_next_token(&p, arena))->type != T_EOF) {
        if (tok->type != T_CHAR) continue;

        // 创建字符NFA片段
        State* mid = create_state(arena);
        State* end = create_state(arena);
        add_transition(arena, mid, tok->value, end);

        // lookahead 判断是否是 STAR
        const char* lookahead = p;
        Token* next_tok = lexer_next_token(&lookahead, arena);
        if (next_tok->type == T_STAR) {
            // 跳过 STAR
            p = lookahead;

            // 添加循环、跳过和连接
            add_transition(arena, current, '\0', mid);  // current -> mid
            add_transition(arena, end, '\0', mid);      // end -> mid
            add_transition(arena, current, '\0', end);  // current -> end
            current = end;
        } else {
            // 非 * 情况，直接连接
            add_transition(arena, current, tok->value, end);
            current = end;
        }
    }

    current->is_accepting = 1;
    return start;
}