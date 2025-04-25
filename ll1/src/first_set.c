#include "first_set.h"

static SymbolSet* get_or_create_set(SymbolSet* sets, 
                                    int* count, 
                                    char symbol, 
                                    Arena* arena){
    // symbol对应的表如果存在就返回这个表
    for(int i = 0; i < *count; i++){
        if(sets[i].symbol == symbol) return &sets[i];
    }
    // symbol对应的表不存在，则创建新表
    SymbolSet* set = &sets[(*count)++];
    set->symbol = symbol;
    set->first = arena_alloc(arena, MAX_SYMBOLS);
    set->follow = arena_alloc(arena, MAX_SYMBOLS);
    if(!set->first || !set->follow){
        fprintf(stderr, "Error: Failed to allocate memory for First or Follow.\n");
        return NULL; 
    }
    set->first[0] = '\0';
    set->follow[0] = '\0';
    return set;
}
static bool add_char(char* set, char c){
    if(strchr(set, c)) return false;
    int len = strlen(set);
    set[len] = c;
    set[len+1] = '\0';
    return true;
}

static void process_right_symbol_first(char* rhs, 
                                       SymbolSet* lhs_set, 
                                       SymbolSet* sets, 
                                       int* set_count, 
                                       bool* changed, 
                                       Arena* arena)
{
    bool epsilon_in_rhs = true;

    for(int j = 0; rhs[j]; j++){
        char symbol = rhs[j];
        if(isspace(symbol)) continue;

        if(is_terminal(symbol) || symbol == '#'){
            if(add_char(lhs_set->first, symbol)) *changed = true;
            epsilon_in_rhs = (symbol == '#');
            break;
        }else{
            SymbolSet* sym_set = get_or_create_set(sets, set_count, symbol, arena);
            for(int k = 0; sym_set->first[k]; k++){
                if(sym_set->first[k] != '#'){
                    if(add_char(lhs_set->first, sym_set->first[k])) *changed = true;
                }
            }
            if(!strchr(sym_set->first, '#')){
                epsilon_in_rhs = false;
                break;
            }
        }
    }
    if(epsilon_in_rhs){
        if(add_char(lhs_set->first, '#')) *changed = true;
    }
}
void compute_first_sets(Grammar* grammar, 
                        SymbolSet* sets, 
                        int* set_count, 
                        Arena* arena){
    bool changed;
    do{
        changed = false;
        for(int i = 0; i < grammar->rule_count; i++){
            Rule* rule = &grammar->rules[i];
            SymbolSet* left_symbol_set = get_or_create_set(sets, set_count, rule->left_hs, arena);
            process_right_symbol_first(rule->right_hs, left_symbol_set, sets, set_count, &changed, arena);
        }
    }while (changed);
    
}