#include "grammar.h"
#include "first_follow.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>  // 用于可变参数处理

/// 打印统一格式的错误信息
static void grammar_report_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "[Grammar Error] ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}

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
        grammar_report_error("Exceeded maximum number of symbols (%d).", GRAMMAR_MAX_SYMBOLS);
        exit(EXIT_FAILURE); 
    }
    list[(*count)++] = c; // 添加新符号
}

/// 初始化 Grammar 结构体，包括规则数组的分配
static Grammar* init_grammar(Arena* arena) {
    Grammar* grammar = arena_alloc(arena, sizeof(Grammar));
    if (!grammar) {
        grammar_report_error("Failed to allocate memory for Grammar struct.");
        return NULL;
    }
    Rule* rules = arena_alloc(arena, GRAMMAR_MAX_RULES * sizeof(Rule));
    if (!rules) {
        grammar_report_error("Failed to allocate memory for rule struct.");
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
            grammar_report_error("Exceeded maximum number of rules (%d).", GRAMMAR_MAX_RULES);
            return false;
        }

        // 构建规则结构体
        Rule* rule = &grammar->rules[grammar->rule_count++];
        rule->left_hs = l_hs;

        size_t token_len = strnlen(token, GRAMMAR_MAX_SYMBOLS);
        rule->right_hs_count = (size_t)(token_len + 1); 
        rule->right_hs = arena_alloc(arena, rule->right_hs_count);
        if (!rule->right_hs) {
            grammar_report_error("Failed to allocate memory for Grammar rules.");
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

/// 提取 LHS 非终结符（必须是箭头左侧的第一个非空字符）
static GrammarStatus grammar_extract_lhs(const char* line, char* lhs) {
    if (!line || !lhs) return GRAMMAR_ERROR_INVALID_ARGUMENT;

    const char* arrow = strstr(line, "->");
    if (!arrow || arrow == line) {
        return GRAMMAR_ERROR_INVALID_FORMAT;
    }
    // 提取 -> 左边的部分
    size_t lhs_len = arrow - line;
    if (lhs_len != 1) {
        return GRAMMAR_ERROR_INVALID_FORMAT; // LHS 必须是单个字符
    }
    char candidate = line[0];
    if (!grammar_is_nonterminal(candidate)) {
        return GRAMMAR_ERROR_INVALID_NONTERMINAL;
    }

    *lhs = candidate;
    return GRAMMAR_OK;
}

// 提取 RHS：即 "->" 之后的部分（去除前导空格），返回状态码
static GrammarStatus grammar_extract_rhs(const char* line, char** rhs_out) {
    if (!line || !rhs_out) return GRAMMAR_ERROR_INVALID_ARGUMENT;

    const char* arrow = strstr(line, "->");
    if (!arrow) return GRAMMAR_ERROR_INVALID_FORMAT;

    arrow += 2; // 跳过 "->"

    // 跳过空白字符
    while (isspace((unsigned char)*arrow)) {
        arrow++;
    }

    if (*arrow == '\0') return GRAMMAR_ERROR_INVALID_FORMAT;

    *rhs_out = (char*)arrow;
    return GRAMMAR_OK;
}

/// 处理文法文件中的一行，解析为规则并添加进文法
static bool grammar_process_line(char* line, Grammar* grammar, Arena* arena) {
    remove_spaces(line);
    if (line[0] == '\0') return true;  // 空行跳过

    char lhs;
    GrammarStatus status = grammar_extract_lhs(line, &lhs);
    if (status != GRAMMAR_OK) {
        grammar_report_error("Invalid LHS in line: %s", line);
        return status;
    }

    char* rhs = NULL;
    status = grammar_extract_rhs(line, &rhs);
    if (status != GRAMMAR_OK) {
        grammar_report_error("Invalid RHS in line: %s", line);
        return status;
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
        grammar_report_error("Failed to open the file.");
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