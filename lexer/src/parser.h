#ifndef PARSER_H
#define PARSER_H

#include "nfa.h"

State* parse_regex(const char* regex, struct Arena* arena);

#endif