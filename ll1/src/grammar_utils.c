#include "grammar.h"


bool is_terminal(char c){
    return islower(c) || !isalpha(c);
}

bool is_nonterminal(char c){
    return isupper(c);
}