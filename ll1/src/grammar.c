#include "grammar.h"
#include "first_follow.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>  // 用于可变参数处理

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
static GrammarResultVoid grammar_add_unique_symbol(char* list, uint8_t* count, char c) {
    if (!list || !count) {
        return (GrammarResultVoid){ .status = GRAMMAR_ERROR_INVALID_ARGUMENT };
    }
    for (uint8_t i = 0; i < *count; i++) {
        if (list[i] == c) {
            return  (GrammarResultVoid){ .status = GRAMMAR_OK };
        }    
    }
    if (*count >= GRAMMAR_MAX_SYMBOLS) {
        grammar_report_error("Exceeded maximum number of symbols (%d).", GRAMMAR_MAX_SYMBOLS);
        return (GrammarResultVoid){ .status = GRAMMAR_ERROR_TOO_MANY_SYMBOLS }; 
    }
    list[(*count)++] = c; // 添加新符号
    return (GrammarResultVoid){ .status = GRAMMAR_OK};
}

/// 初始化 Grammar 结构体，包括规则数组的分配
static GrammarResultGrammar init_grammar(Arena* arena) {
    if(!arena) {
        return (GrammarResultGrammar){. status = GRAMMAR_ERROR_ALLOCATION_FAILED};
    }
    Grammar* grammar = arena_alloc(arena, sizeof(Grammar));
    if (!grammar) {
        grammar_report_error("Failed to allocate memory for Grammar struct.");
        return (GrammarResultGrammar){. status = GRAMMAR_ERROR_ALLOCATION_FAILED};
    }
    Rule* rules = arena_alloc(arena, GRAMMAR_MAX_RULES * sizeof(Rule));
    if (!rules) {
        grammar_report_error("Failed to allocate memory for rule struct.");
        return (GrammarResultGrammar){. status = GRAMMAR_ERROR_ALLOCATION_FAILED};
    }
    grammar->rules = rules;
    grammar->rule_count = 0;
    grammar->nonterminals_count = 0;
    grammar->terminals_count = 0;

    GRAMMAR_DEBUG("Grammar structure initialized.");
    return (GrammarResultGrammar){. status = GRAMMAR_OK,
                                  . value = grammar};
}

/// 去除字符串中的所有空白字符（就地修改）
static void line_remove_spaces(char* str) {
    char* dst = str;
    for (char* src = str; *src != '\0'; src++) {
        if (!isspace((unsigned char)*src)) {
            *dst++ = *src;
        }
    }
    *dst = '\0';
}

/// 收集产生式右部出现的所有符号并添加到文法中（终结符、非终结符）
static GrammarResultVoid grammar_collect_rhs_symbols(char* rhs, Grammar* grammar) {
    for (char* p = rhs; *p; ++p) {
        GrammarResultVoid res;
        if (*p == '#') {
            res = grammar_add_unique_symbol(grammar->terminals, &grammar->terminals_count, '#');
            if (res.status != GRAMMAR_OK) return res;
            GRAMMAR_DEBUG("Collected terminal: #");
        } else if (grammar_is_nonterminal(*p)) {
            res = grammar_add_unique_symbol(grammar->nonterminals, &grammar->nonterminals_count, *p);
            if (res.status != GRAMMAR_OK) return res;
            GRAMMAR_DEBUG("Collected nonterminal: %c", *p);
        } else if (grammar_is_terminal(*p)) {
            res = grammar_add_unique_symbol(grammar->terminals, &grammar->terminals_count, *p);
            if (res.status != GRAMMAR_OK) return res;
            GRAMMAR_DEBUG("Collected terminal: %c", *p);
        }
    }
    return (GrammarResultVoid){ .status = GRAMMAR_OK };
}

/// 解析右部多个产生式（用 | 分隔），为每个产生式构建规则结构体
static GrammarResultVoid grammar_parse_rhs(Grammar* grammar, char l_hs, char* r_hs, Arena* arena) {
    if (!grammar || !r_hs || !arena)
        return (GrammarResultVoid){ .status = GRAMMAR_ERROR_INVALID_ARGUMENT };

    char* rhs_copy = arena_alloc(arena, strlen(r_hs) + 1);
    if (!rhs_copy) {
        grammar_report_error("Failed to allocate memory for RHS copy.");
        return (GrammarResultVoid){ .status = GRAMMAR_ERROR_ALLOCATION_FAILED };
    }
    strcpy(rhs_copy, r_hs);

    char* saveptr;
    char* token = strtok_r(rhs_copy, "|\n", &saveptr);
    while (token) {
        while (isspace(*token)) token++;

        if (grammar->rule_count >= GRAMMAR_MAX_RULES) {
            grammar_report_error("Exceeded maximum number of rules (%d).", GRAMMAR_MAX_RULES);
            return (GrammarResultVoid){ .status = GRAMMAR_ERROR_TOO_MANY_RULES };
        }

        Rule* rule = &grammar->rules[grammar->rule_count++];
        rule->left_hs = l_hs;

        size_t token_len = strnlen(token, GRAMMAR_MAX_SYMBOLS);
        rule->right_hs = arena_alloc(arena, token_len + 1);
        rule->right_hs_count = token_len;
        if (!rule->right_hs) {
            grammar_report_error("Failed to allocate memory for Grammar rules.");
            return (GrammarResultVoid){ .status = GRAMMAR_ERROR_ALLOCATION_FAILED };
        }

        strncpy(rule->right_hs, token, token_len);
        rule->right_hs[token_len] = '\0';
        GRAMMAR_DEBUG("Added rule: %c -> %s", l_hs, rule->right_hs);

        GrammarResultVoid sym_res = grammar_collect_rhs_symbols(token, grammar);
        if (sym_res.status != GRAMMAR_OK) return sym_res;

        token = strtok_r(NULL, "|\n", &saveptr);
    }

    return (GrammarResultVoid){ .status = GRAMMAR_OK };
}

/// 提取 LHS 非终结符（必须是箭头左侧的第一个非空字符）
static GrammarResultChar grammar_extract_lhs(const char* line) {
    if (!line) return (GrammarResultChar){ .status = GRAMMAR_ERROR_INVALID_ARGUMENT };

    const char* arrow = strstr(line, "->");
    if (!arrow || arrow == line) return (GrammarResultChar){ .status = GRAMMAR_ERROR_INVALID_FORMAT };

    size_t lhs_len = arrow - line;
    if (lhs_len != 1) return (GrammarResultChar){ .status = GRAMMAR_ERROR_INVALID_FORMAT };

    char candidate = line[0];
    if (!grammar_is_nonterminal(candidate)) {
        return (GrammarResultChar){ .status = GRAMMAR_ERROR_INVALID_NONTERMINAL };
    }

    return (GrammarResultChar){ .status = GRAMMAR_OK, .value = candidate };
}

// 提取 RHS：即 "->" 之后的部分（去除前导空格），返回状态码
static GrammarResultString grammar_extract_rhs(const char* line) {
    if (!line) return (GrammarResultString){ .status = GRAMMAR_ERROR_INVALID_ARGUMENT };

    const char* arrow = strstr(line, "->");
    if (!arrow) return (GrammarResultString){ .status = GRAMMAR_ERROR_INVALID_FORMAT };

    arrow += 2;
    while (isspace((unsigned char)*arrow)) arrow++;
    if (*arrow == '\0') return (GrammarResultString){ .status = GRAMMAR_ERROR_INVALID_FORMAT };

    return (GrammarResultString){ .status = GRAMMAR_OK, .value = (char*)arrow };
}

/// 处理文法文件中的一行，解析为规则并添加进文法
static GrammarResultVoid grammar_process_line(char* line, Grammar* grammar, Arena* arena) {
    line_remove_spaces(line);
    if (line[0] == '\0') return (GrammarResultVoid){ .status = GRAMMAR_OK };

    GrammarResultChar lhs_res = grammar_extract_lhs(line);
    if (lhs_res.status != GRAMMAR_OK) {
        grammar_report_error("Invalid LHS in line: %s", line);
        return (GrammarResultVoid){ .status = lhs_res.status };
    }

    GrammarResultString rhs_res = grammar_extract_rhs(line);
    if (rhs_res.status != GRAMMAR_OK) {
        grammar_report_error("Invalid RHS in line: %s", line);
        return (GrammarResultVoid){ .status = rhs_res.status };
    }

    GrammarResultVoid add_res = grammar_add_unique_symbol(grammar->nonterminals, &grammar->nonterminals_count, lhs_res.value);
    if (add_res.status != GRAMMAR_OK) return add_res;

    GrammarResultVoid parse_res = grammar_parse_rhs(grammar, lhs_res.value, rhs_res.value, arena);
    if (parse_res.status != GRAMMAR_OK) return parse_res;

    GRAMMAR_DEBUG("Processed line successfully: %s", line);
    return (GrammarResultVoid){ .status = GRAMMAR_OK };
}

/// 从文件读取文法定义，解析并构建 Grammar 结构
GrammarResultGrammar read_grammar(const char* filename, Arena* arena) {
    GRAMMAR_DEBUG("Reading grammar from file: %s", filename);
    GrammarResultGrammar init_res = init_grammar(arena);
    if (init_res.status != GRAMMAR_OK) return init_res;

    Grammar* grammar = init_res.value;
    FILE* file = fopen(filename, "r");
    if (!file) {
        grammar_report_error("Failed to open the file.");
        return (GrammarResultGrammar){ .status = GRAMMAR_ERROR_IO_FAILED };
    }

    char line[GRAMMAR_MAX_LINE_LEN];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = '\0';
        GrammarResultVoid line_res = grammar_process_line(line, grammar, arena);
        if (line_res.status != GRAMMAR_OK) {
            fclose(file);
            return (GrammarResultGrammar){ .status = line_res.status };
        }
    }

    fclose(file);
    GRAMMAR_DEBUG("Finished reading grammar file.");
    return (GrammarResultGrammar){ .status = GRAMMAR_OK, .value = grammar };
}
