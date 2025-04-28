// arena.h
#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

typedef struct Arena {
    char* buffer;
    size_t size;
    size_t offset;
}Arena;

Arena* arena_create(size_t size);
void* arena_alloc(Arena* arena, size_t size);
void arena_free(Arena* arena);

#endif // ARENA_H