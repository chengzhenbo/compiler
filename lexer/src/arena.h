// arena.h
#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

struct Arena;  // 前向声明，无 typedef

struct Arena* arena_create(size_t size);
void* arena_alloc(struct Arena* arena, size_t size);
void arena_free(struct Arena* arena);

#endif // ARENA_H