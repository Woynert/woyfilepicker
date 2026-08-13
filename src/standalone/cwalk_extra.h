#ifndef CWALK_EXTRA_H
#define CWALK_EXTRA_H

#include "arena.h"
#include "arena_extra.h"
#include "cwalk.h"
#include "strview.h"
#include "wstrview.h"
#include "raylib.h"
#include "raylib_extra.h"
#include "strbuf_extra.h"

void wcwk_path_join(strview_t path_a, strview_t path_b, strbuf_t **out_path, Arena *arena) {
    strbuf_t *a_buf = strbuf_create_with_arena(path_a, arena);
    strbuf_t *b_buf = strbuf_create_with_arena(path_b, arena);
    size_t required_size = 1+cwk_path_join(a_buf->cstr, b_buf->cstr, NULL, 0);
    strbuf_grow(out_path, (int)required_size);
    cwk_path_join(a_buf->cstr, b_buf->cstr, (*out_path)->cstr, required_size);
    strbuf_update_cstr_size(out_path, (int)required_size);
}

void wcwk_path_get_relative(strview_t from, strview_t to, strbuf_t **out_path, Arena *arena) {
    strbuf_t *from_buf = strbuf_create_with_arena(from, arena);
    strbuf_t *to_buf = strbuf_create_with_arena(to, arena);
    size_t required_size = 1+cwk_path_get_relative(from_buf->cstr, to_buf->cstr, NULL, 0);
    strbuf_grow(out_path, (int)required_size);
    cwk_path_get_relative(from_buf->cstr, to_buf->cstr, (*out_path)->cstr, required_size);
    strbuf_update_cstr_size(out_path, (int)required_size);
}

/// @Returns View from inside file_path_view.
/// @Retval STRVIEW_INVALID on Error.
strview_t get_dir_from_file_path_and_check_exists(const strview_t file_path_view, Arena scratch) {
    strbuf_t *dir = strbuf_create_with_arena(file_path_view, &scratch);
    if (!IsPathFile(dir->cstr)) {
        printfd("ERR: Path is not file "PRIstrw, PRIstrarg(file_path_view));
        return STRVIEW_INVALID;
    }
    size_t dir_size;
    cwk_path_get_dirname(dir->cstr, &dir_size);
    strbuf_update_cstr_size(&dir, (int)dir_size);
    if (!IsPathDirectory_arena(strbuf_view2(dir), scratch)) {
        printfd("ERR: Not a directory.");
        return STRVIEW_INVALID;
    }
    return (strview_t) { .data = file_path_view.data, .size = dir->size };
}


#endif
