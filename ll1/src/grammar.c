#include "grammar.h"

void add_unique_nonterminals(char* list, int* count, char c)
{
    for(int i = 0; i < *count; i++){
        if(list[i] == c) return;
    }
    list[(*count)++] = c;
}

Grammar* read_grammar(const char* filename, Arena* arena){
    FILE* file = fopen(filename, "r");
    if (!file){
        fprintf(stderr, "Error: Failed to open the file.\n");
        return NULL;
    }
    Grammar* grammar = arena_alloc(arena, sizeof(Grammar));
    if(!grammar){
        fprintf(stderr, "Error: Failed to allocate memory for Grammar struct.\n");
        return NULL; // 内存分配失败
    }
    Rule* rules = arena_alloc(arena, MAX_RULES*sizeof(Rule));
    if(!rules){
        fprintf(stderr, "Error: Failed to allocate memory for rule struct.\n");
        return NULL;
    }
    grammar->rules = rules;
    grammar->rule_count = 0;
    grammar->nondermials_count = 0;
    grammar->dermials_count = 0;

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        //忽略空行
        if(line[0] == '\n' ) continue;
        //根据箭头来区分文法的左部和右部
        char* arrow = strstr(line, "->");
        if(!arrow || arrow == line) continue; //没有箭头或者出现在头部

        *arrow = '\0';
        char l_hs = line[0];
        char* r_hs = arrow+2;
        //将非终结符加入到文法中
        add_unique_nonterminals(grammar->nonderminals, &grammar->nondermials_count, l_hs);

        char* token = strtok(r_hs, "|\n");

        while (token != NULL) {
            while(isspace(*token)) token++;

            Rule* rule = &grammar->rules[grammar->rule_count++];
            rule->left_hs = l_hs;
            rule->right_hs = arena_alloc(arena, strlen(token)+1);
            strcpy(rule->right_hs, token);

            for(char* p = token; *p; ++p){
                if(isspace(*p)) continue;
                if(*p == '#'){
                    add_unique_nonterminals(grammar->derminals, &grammar->dermials_count, '#');
                }else if(isupper(*p)){
                    add_unique_nonterminals(grammar->nonderminals, &grammar->nondermials_count, *p);
                }else{
                    add_unique_nonterminals(grammar->derminals, &grammar->dermials_count, *p);
                }
            }

            token = strtok(NULL, "|\n");
        }
        
    }
    
    fclose(file);
    return grammar;
}