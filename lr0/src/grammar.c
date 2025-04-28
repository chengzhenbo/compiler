#include "grammar.h"

static bool is_terminal(char c){
    return islower(c) || !isalpha(c);
}

static bool is_nonterminal(char c){
    return isupper(c);
}
static void add_unique_nonterminals(char* list, int* count, char c)
{
    //过滤重复的字符
    for(int i = 0; i < *count; i++){
        if(list[i] == c) return;
    }
    //list中仅添加新字符c
    list[(*count)++] = c;
}

void collect_symbols_from_rhs(char* rhs, Grammar* grammar) {
    for (char* p = rhs; *p; ++p) {
        if (isspace(*p)) continue;
        if (*p == '#') { //#表示空串
            add_unique_nonterminals(grammar->terminals, &grammar->termials_count, '#');
        } else if (is_nonterminal(*p)) {
            add_unique_nonterminals(grammar->nonterminals, &grammar->nontermials_count, *p);
        } else if (is_terminal(*p)) {
            add_unique_nonterminals(grammar->terminals, &grammar->termials_count, *p);
        }
    }
}

Grammar* read_grammar(const char* filename, Arena* arena){
    
    Grammar* grammar = arena_alloc(arena, sizeof(Grammar));
    if(!grammar){
        fprintf(stderr, "Error: Failed to allocate memory for Grammar struct.\n");
        return NULL; 
    }
    Rule* rules = arena_alloc(arena, MAX_RULES*sizeof(Rule));
    if(!rules){
        fprintf(stderr, "Error: Failed to allocate memory for rule struct.\n");
        return NULL;
    }
    grammar->rules = rules;
    grammar->rule_count = 0;
    grammar->nontermials_count = 0;
    grammar->termials_count = 0;

    FILE* file = fopen(filename, "r");
    if (!file){
        fprintf(stderr, "Error: Failed to open the file.\n");
        return NULL;
    }
    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        //忽略空行
        if (line[0] == '\0' || isspace(line[0])) continue;
        //根据箭头来区分文法的左部和右部
        char* arrow = strstr(line, "->");
        if(!arrow || arrow == line) continue; //没有箭头或者出现在头部

        *arrow = '\0';
        char l_hs = line[0];
        char* r_hs = arrow+2;
        //将非终结符加入到文法中
        add_unique_nonterminals(grammar->nonterminals, 
                                &grammar->nontermials_count, 
                                l_hs);

        //将产生式右部按｜进行分解后与非终结符构成一条产生式
        char* token = strtok(r_hs, "|\n");
        while (token != NULL) {
            while(isspace(*token)) token++;
            //添加每一个产生式到文法
            Rule* rule = &grammar->rules[grammar->rule_count++];
            rule->left_hs = l_hs;
            rule->right_hs = arena_alloc(arena, strlen(token)+1);
            if(!rule->right_hs){
                fprintf(stderr, "Error: Failed to allocate memory for Grammar rules.\n");
                fclose(file);
                return NULL; 
            }
            strcpy(rule->right_hs, token);
            
            //添加产生式右部出现的终结符和非终结符到文法
            collect_symbols_from_rhs(token, grammar);

            token = strtok(NULL, "|\n");
        }
    }
    fclose(file);
    return grammar;
}
void grammar_free(Grammar* grammar) {
    if (!grammar) return;
    memset(grammar, 0, sizeof(Grammar));
}