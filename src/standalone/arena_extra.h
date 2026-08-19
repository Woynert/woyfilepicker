#ifndef ARENA_EXTRA_H
#define ARENA_EXTRA_H

#include "../subprojects/woycontainer/src/arena.h"
#include "strbuf.h"

static void* arena_strbuf_allocator(
    strbuf_allocator_t* this_allocator, void* ptr, size_t size
) {
    // New allocation: ptr == NULL && size > 0
    // Reallocation:   ptr != NULL && size > 0
    // Free:           ptr != NULL && size == 0

    if (size == 0) return NULL; // No freeing for arena.

    Arena *arena = (Arena*)this_allocator->app_data;
    void *result = arena_new_align(arena, char, ptrdiff_t, (ptrdiff_t)size);

    if (ptr != NULL) { // Reallocation must copy what we had.
        // Reallocation must copy what we had.
        // Don't use memcpy.
        memmove(result, ptr, size);
        /*
           TODO: Check if there would be overlap in this operation and
           print the delta. If there's overlap that would indicate there's
           a bug with the arena_alloc I think.
        */
    }

    return result;
}

static strbuf_allocator_t make_arena_strbuf_allocator(Arena *arena) {
    return (strbuf_allocator_t) { .app_data = arena, .allocator = &arena_strbuf_allocator };
}

static void* arena_allocator(void* ptr, size_t size, int align, void* user_data) {
    // New allocation: ptr == NULL && size > 0
    // Reallocation:   ptr != NULL && size > 0
    // Free:           ptr != NULL && size == 0

    if (size == 0) return NULL; // No freeing for arena.

    Arena *arena = (Arena*)user_data;
    void *result = arena_alloc(arena, sizeof(char), align, (i64)size);

    if (ptr != NULL) {
        // Reallocation must copy what we had.
        // Don't use memcpy.
        memmove(result, ptr, size);
    }

    return result;
}

#define strbuf_create_with_arena(init, alloc) _Generic((init),\
    size_t   : strbuf_create_with_arena_empty,\
    int      : strbuf_create_with_arena_empty,\
    strview_t: strbuf_create_with_arena_init\
)(init, alloc)


strbuf_t* strbuf_create_with_arena_empty(size_t initial_capacity, Arena *arena) {
    if (arena == NULL) { return NULL; }
    if (initial_capacity > INT_MAX) { return NULL; }
    strbuf_allocator_t allocator = { .app_data = arena, .allocator = arena_strbuf_allocator };
    strbuf_t *buf = (strbuf_t*)allocator.allocator(&allocator, NULL, sizeof(strbuf_t)+initial_capacity+1);
    buf->capacity = (int)initial_capacity;
    buf->allocator = allocator;
    buf->size = 0;
    buf->cstr[0] = 0;
    return buf;
}

strbuf_t* strbuf_create_with_arena_init(strview_t initial_content, Arena *arena) {
    if (arena == NULL) { return NULL; }
    if (initial_content.size < 0) { return NULL; }
    strbuf_t *buf = strbuf_create_with_arena_empty((size_t)initial_content.size, arena);
    strbuf_assign(&buf, initial_content);
    return buf;
}

// Quick string format.
__attribute__((format(printf, 2, 3)))
strview_t SF(Arena *perm, const char *fmt, ...) {
    strbuf_t *buf = strbuf_create_with_arena(0, perm);
    va_list va;
    strview_t str = STRVIEW_INVALID;
    va_start(va, fmt);
    str = strbuf_vprintf(&buf, fmt, va);
    va_end(va);
    return str;
}

// Quick string concatenation.
#define SC(perm, ...) _SC(perm, PP_NARG(__VA_ARGS__), __VA_ARGS__)
strview_t _SC(Arena *perm, int n_args, ...) {
    strbuf_t *buf = strbuf_create_with_arena(0, perm);
    va_list va;
    va_start(va, n_args);
    strview_t str = STRVIEW_INVALID;
    str = strbuf_vcat(&buf, n_args, va);
    va_end(va);
    return str;
}


#endif // !ARENA_EXTRA_H
