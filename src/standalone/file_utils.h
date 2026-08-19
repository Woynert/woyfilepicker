#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include "arena.h"
#include "arena_extra.h"
#include "stdbool.h"
#include "strbuf.h"
#include "strview.h"
#include <sys/stat.h>

// @Note. Beware of this behaviour:
//        /my/dir/ == False. Not a directory.
//        /my/dir  == True. It's a directory.
bool is_path_dir_cstr(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) { return false; }
    return path_stat.st_mode & __S_IFDIR;
}

bool is_path_dir(strview_t path, Arena scratch) {
    if (!strview_is_valid(path)) { return false; }
    while (path.size > 0 && path.data[path.size] == '/') { --path.size; }
    strbuf_t *path_buf = strbuf_create_with_arena(path, &scratch);
    return is_path_dir_cstr(path_buf->cstr);
}

bool is_path_file_cstr(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) { return false; }
    return path_stat.st_mode & __S_IFREG;
}

bool is_path_file(strview_t path, Arena scratch) {
    strbuf_t *path_buf = strbuf_create_with_arena(path, &scratch);
    return is_path_file_cstr(path_buf->cstr);
}

/*
bool path_exists_and_get_if_is_dir(Arena scratch, strview_t path, bool *out_is_dir) {
    if (!strview_is_valid(path)) { return false; }
    while (path.size > 0 && path.data[path.size] == '/') { --path.size; }
    strbuf_t *path_buf = strbuf_create_with_arena(path, &scratch);
    struct stat path_stat;
    if (stat(path_buf->cstr, &path_stat) != 0) { return false; }
    *out_is_dir = path_stat.st_mode & __S_IFDIR;
    return true;
}
*/

#endif
