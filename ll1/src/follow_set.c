#include "follow_set.h"
static void propagate_follow(Grammar* grammar, 
                             SymbolSet* sets, 
                             int* set_count, 
                             bool* changed,
                             Arena* arena)
{
    for(int i = 0; i < grammar->rule_count; i++){
        Rule* rule = &grammar->rules[i];
        size_t len_rule = strlen(rule->right_hs);

        for(size_t j = 0; j < len_rule; j++){
            char B = rule->right_hs[j];
            if(!is_nonterminal(B)) continue;
            SymbolSet* B_set = get_or_create_set(sets, set_count, B, arena);

            bool epsilon_chain = true; // epsilon 闭包运输
            for(size_t k = j+1; k < len_rule; k++){
                char next_sym = rule->right_hs[k];
                //非终结符B后紧跟着终结符，就将其加入到B的follow集
                if(is_terminal(next_sym)){
                    if(add_char(B_set->follow, next_sym)) *changed = true;
                    epsilon_chain = false;
                    break;
                }
                //B后可能跟着非终结符，将该终结符first集/epsilon的符合加入到B的follow集
                SymbolSet* next_sym_set = get_or_create_set(sets, set_count, next_sym, arena);
                for(size_t m = 0; next_sym_set->first[m]; m++){
                    if(next_sym_set->first[m] != '#'){
                        if(add_char(B_set->follow, next_sym_set->first[m])) *changed = true;
                    }
                }
                //B后面字符first集没有空串，就不需要做闭包运算
                if(!strchr(next_sym_set->first, '#')){
                    epsilon_chain = false;
                    break;
                }
            }
            //闭包运算
            if(epsilon_chain){
                SymbolSet* A_set = get_or_create_set(sets, set_count, rule->left_hs, arena);
                for(size_t n = 0; A_set->follow[n]; n++){
                    if(add_char(B_set->follow, A_set->follow[n])) *changed = true;
                }
            }
        }
    }
}

void compute_follow_sets(Grammar* grammar, 
                         SymbolSet* sets, 
                         int* set_count, 
                         Arena* arena)
{
    SymbolSet* start = get_or_create_set(sets, set_count, 'S', arena);
    add_char(start->follow, '$');
    bool changed;
    do{
        changed = false;
        propagate_follow(grammar, sets, set_count, &changed, arena);
    }while(changed);
}
