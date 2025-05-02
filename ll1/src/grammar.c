#include "grammar.h"
#include "first_follow.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

/// 判断是否为终结符：小写字母或非字母字符
bool grammar_is_terminal(char c) {
    return islower(c) || !isalpha(c);
}

/// 判断是否为非终结符：大写字母
bool grammar_is_nonterminal(char c) {
    return isupper(c);
}

/// 将符号 c 添加到符号列表中，避免重复，超出上限时报错
static void grammar_add_unique_symbol(char* list, uint8_t* count, char c) {
    for (uint8_t i = 0; i < *count; i++) {
        if (list[i] == c) return; // 已存在
    }
    if (*count >= GRAMMAR_MAX_SYMBOLS) {
        fprintf(stderr, "Error: Exceeded maximum number of symbols (%d).\n", GRAMMAR_MAX_SYMBOLS);
        exit(EXIT_FAILURE); 
    }
    list[(*count)++] = c; // 添加新符号
}

/// 检查左部是否合法（必须是单个非终结符）
static bool grammar_is_valid_lhs(const char* lhs) {
    return lhs && grammar_is_nonterminal(lhs[0]);
}

/// 初始化 Grammar 结构体，包括规则数组的分配
static Grammar* init_grammar(Arena* arena) {
    Grammar* grammar = arena_alloc(arena, sizeof(Grammar));
    if (!grammar) {
        fprintf(stderr,"Failed to allocate memory for Grammar struct.");
        return NULL;
    }
    Rule* rules = arena_alloc(arena, GRAMMAR_MAX_RULES * sizeof(Rule));
    if (!rules) {
        fprintf(stderr,"Failed to allocate memory for rule struct.");
        return NULL;
    }
    grammar->rules = rules;
    grammar->rule_count = 0;
    grammar->nonterminals_count = 0;
    grammar->terminals_count = 0;
    return grammar;
}

/// 去除字符串中的所有空白字符（就地修改）
static void remove_spaces(char* str) {
    char* dst = str;
    for (char* src = str; *src != '\0'; src++) {
        if (!isspace((unsigned char)*src)) {
            *dst++ = *src;
        }
    }
    *dst = '\0';
}

/// 收集产生式右部出现的所有符号并添加到文法中（终结符、非终结符）
void grammar_collect_rhs_symbols(char* rhs, Grammar* grammar) {
    for (char* p = rhs; *p; ++p) {
        if (*p == '#') { // '#' 表示空串
            grammar_add_unique_symbol(grammar->terminals, &grammar->terminals_count, '#');
        } else if (grammar_is_nonterminal(*p)) {
            grammar_add_unique_symbol(grammar->nonterminals, &grammar->nonterminals_count, *p);
        } else if (grammar_is_terminal(*p)) {
            grammar_add_unique_symbol(grammar->terminals, &grammar->terminals_count, *p);
        }
    }
}

/// 解析右部多个产生式（用 | 分隔），为每个产生式构建规则结构体
static bool grammar_parse_rhs(Grammar* grammar, char l_hs, char* r_hs, Arena* arena) {
    char* saveptr;
    char* token = strtok_r(r_hs, "|\n", &saveptr); // 分割多个右部产生式

    while (token) {
        while (isspace(*token)) token++; // 跳过前导空白

        if (grammar->rule_count >= GRAMMAR_MAX_RULES) {
            fprintf(stderr, "Error: Exceeded maximum number of rules (%d).\n", GRAMMAR_MAX_RULES);
            return false;
        }

        // 构建规则结构体
        Rule* rule = &grammar->rules[grammar->rule_count++];
        rule->left_hs = l_hs;

        size_t token_len = strnlen(token, GRAMMAR_MAX_SYMBOLS);
        rule->right_hs_count = (size_t)(token_len + 1); 
        rule->right_hs = arena_alloc(arena, rule->right_hs_count);
        if (!rule->right_hs) {
            fprintf(stderr, "Error: Failed to allocate memory for Grammar rules.\n");
            return false;
        }

        strncpy(rule->right_hs, token, token_len);
        rule->right_hs[token_len] = '\0'; // 添加字符串结束符

        // 收集右部符号
        grammar_collect_rhs_symbols(token, grammar);

        token = strtok_r(NULL, "|\n", &saveptr); // 处理下一个右部
    }   
    return true; 
}

/// 从一行中提取产生式左部符号和右部字符串
static bool grammar_extract_lhs_rhs(char* line, char* lhs, char** rhs) {
    char* arrow = strstr(line, "->");
    if (!arrow || arrow == line) return false;

    *lhs = line[0];
    *rhs = arrow + 2; // 指向右部起始位置
    return true;
}

/// 处理文法文件中的一行，解析为规则并添加进文法
static bool grammar_process_line(char* line, Grammar* grammar, Arena* arena) {
    remove_spaces(line);
    if (line[0] == '\0') return true;  // 空行跳过

    char lhs;
    char* rhs;

    if (!grammar_extract_lhs_rhs(line, &lhs, &rhs)) {
        fprintf(stderr, "Error: Invalid grammar line format: %s\n", line);
        return false;
    }

    if (!grammar_is_valid_lhs(&lhs)) {
        fprintf(stderr, "Error: Invalid LHS nonterminal: %c\n", lhs);
        return false;
    }

    grammar_add_unique_symbol(grammar->nonterminals, &grammar->nonterminals_count, lhs);

    return grammar_parse_rhs(grammar, lhs, rhs, arena);
}

/// 从文件读取文法定义，解析并构建 Grammar 结构
Grammar* read_grammar(const char* filename, Arena* arena) {
    Grammar* grammar = init_grammar(arena);
    if (!grammar) return NULL;

    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Failed to open the file.\n");
        return NULL;
    }

    char line[GRAMMAR_MAX_LINE_LEN];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = '\0';  // 去除换行符
        if (!grammar_process_line(line, grammar, arena)) {
            fclose(file);
            return NULL;
        }
    }

    fclose(file);
    return grammar;
}
void print_grammar(const Grammar* grammar) {
    printf("=== Grammar ===\n");

    for (size_t i = 0; i < grammar->rule_count; ++i) {
        const Rule* rule = &grammar->rules[i];
        printf("%c -> %s\n", rule->left_hs, rule->right_hs);
    }

    printf("\nNonterminals (%d): ", grammar->nonterminals_count);
    for (uint8_t i = 0; i < grammar->nonterminals_count; ++i) {
        printf("%c ", grammar->nonterminals[i]);
    }

    printf("\nTerminals (%d): ", grammar->terminals_count);
    for (uint8_t i = 0; i < grammar->terminals_count; ++i) {
        printf("%c ", grammar->terminals[i]);
    }
    printf("\n================\n");
}