#ifndef MATCHER_H
#define MATCHER_H

#include "nfa.h"

int simulate_nfa(State* start, const char* input, struct Arena* arena);

#endif