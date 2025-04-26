#include "first_set.h"

static void process_right_symbol_first(char* rhs, 
                                       SymbolSet* lhs_set, 
                                       SymbolSet* sets, 
                                       int* set_count, 
                                       bool* changed, 
                                       Arena* arena)
{
    bool epsilon_in_rhs = true;

    for(size_t j = 0; rhs[j]; j++){
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
            SymbolSet* left_symbol_set = get_or_create_set(sets, 
                                                           set_count, 
                                                           rule->left_hs, 
                                                           arena);
            process_right_symbol_first(rule->right_hs, 
                                       left_symbol_set, 
                                       sets, 
                                       set_count, 
                                       &changed, 
                                       arena);
        }
    }while (changed);
}