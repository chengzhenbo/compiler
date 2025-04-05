// arena.c
#include <stdio.h>
#include <stdlib.h>
#include "arena.h"

struct Arena {
    char* buffer;
    size_t size;
    size_t offset;
};

struct Arena* arena_create(size_t size) {
    struct Arena* arena = (struct Arena*)malloc(sizeof(struct Arena));
    if (!arena) {
        fprintf(stderr, "Failed to allocate Arena\n");
        return NULL;
    }
    arena->buffer = (char*)malloc(size);
    if (!arena->buffer) {
        fprintf(stderr, "Failed to allocate Arena buffer\n");
        free(arena);
        return NULL;
    }
    arena->size = size;
    arena->offset = 0;
    return arena;
}

void* arena_alloc(struct Arena* arena, size_t size) {
    if (arena == NULL || arena->buffer == NULL) {
        fprintf(stderr, "Invalid Arena\n");
        return NULL;
    }
    if (arena->offset + size > arena->size) {
        fprintf(stderr, "Arena out of memory\n");
        return NULL;
    }
    void* ptr = arena->buffer + arena->offset;
    arena->offset += size;
    return ptr;
}

void arena_free(struct Arena* arena) {
    if (arena) {
        free(arena->buffer);
        free(arena);
    }
}