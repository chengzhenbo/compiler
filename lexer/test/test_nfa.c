#include <stdio.h>

#include "../src/nfa.h"
#include "../src/nfa.c"

#include "../src/arena.h"
#include "../src/arena.c"

#define MAX_STATES 1024

int get_state_id(State* state, State** id_map, int* next_id) {
    for (int i = 0; i < *next_id; ++i) {
        if (id_map[i] == state) return i;
    }
    if (*next_id >= MAX_STATES) return -1;
    id_map[*next_id] = state;
    return (*next_id)++;
}
// 打印 NFA 状态
void print_state(State* state, int* visited, State** id_map, int* next_id) {
    int id = get_state_id(state, id_map, next_id);
    if (id == -1 || visited[id]) return;
    visited[id] = 1;

    printf("State[%d] %s\n", id, state->is_accepting ? "(accepting)" : "");

    for (Transition* t = state->transitions; t != NULL; t = t->next) {
        int to_id = get_state_id(t->target, id_map, next_id);
        const char* symbol_str = (t->symbol == '\0') ? "\\0" : (char[]){t->symbol, '\0'};
        printf("  --[%s]--> State[%d]\n", symbol_str, to_id);
    }

    for (Transition* t = state->transitions; t != NULL; t = t->next) {
        print_state(t->target, visited, id_map, next_id);
    }
}

int main() {
    struct Arena* arena = arena_create(2048);

    // 第一个测试：a*
    printf("NFA for a*:\n");
    NFA nfa1 = create_star_nfa(arena, create_char_nfa(arena, 'a'));
    int visited1[MAX_STATES] = {0};
    State* id_map1[MAX_STATES] = {0};
    int next_id1 = 0;
    print_state(nfa1.start, visited1, id_map1, &next_id1);

    printf("\n");

    // 第二个测试：ab*
    // 即构建 a + b* 的连接
    printf("NFA for ab*:\n");
    NFA a = create_char_nfa(arena, 'a');
    NFA b = create_char_nfa(arena, 'b');
    NFA b_star = create_star_nfa(arena, b);
    NFA ab_star = create_concat_nfa(arena, a, b_star);

    int visited2[MAX_STATES] = {0};
    State* id_map2[MAX_STATES] = {0};
    int next_id2 = 0;
    print_state(ab_star.start, visited2, id_map2, &next_id2);

    arena_free(arena);
    return 0;
}