#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "state.h"
#include "stdio.h"
#include "stdlib.h"
#include <sys/stat.h>
#include "wod_drawer.h"

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
    if (path.data[path.size] == '/') { --path.size; }
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


void get_env_vars(Ctx *ctx) {
    const char* home = getenv("HOME");
    strview_t home_view = home ? cstr(home) : cstr_SL("/");
    strbuf_assign(&ctx->home, home_view);
    const char* config = getenv("XDG_CONFIG_HOME");
    if (config) {
        strbuf_assign(&ctx->config, cstr(config));
    } else {
        strbuf_cat(&ctx->config, strbuf_view2(ctx->home), cstr_SL("/.config"));
    }
    // Make sure these don't end in '/'.
    if (ctx->home->cstr[ctx->home->size] == '/') { ctx->home->cstr[ctx->home->size]=0; }
    if (ctx->config->cstr[ctx->config->size] == '/') { ctx->config->cstr[ctx->config->size]=0; }
}

void free_file(File *file) {
    strpool_remove(file->strpool, file->path);
    strpool_remove(file->strpool, file->bookmark_display_name);
}

File make_file(Ctx *ctx, strview_t path, Strpool *strpool) {
    File file = { 0 };
    StrpoolId path_str_id = strpool_append(strpool, path);
    if (path_str_id == -1) { goto exit; }
    file.strpool = strpool;
    strbuf_t *path_buf = strbuf_create_with_arena(path, &ctx->framearena);
    struct stat path_stat;
    if (stat(path_buf->cstr, &path_stat) != 0) { goto exit; }
    file.is_dir = path_stat.st_mode & __S_IFDIR;
    struct tm *mod_date = localtime(&path_stat.st_mtime);
    file.mod_date = *mod_date;
    file.valid = true;
    file.path = path_str_id;
    exit: return file;
}

void parse_gtk3_bookmarks(Ctx *ctx) {
    // ~/.config/gtk-3.0/bookmarks
    strbuf_t* bookmarks_path = strbuf_create_with_arena(0, &ctx->framearena);
    strbuf_cat(&bookmarks_path, strbuf_view2(ctx->config), cstr_SL("/gtk-3.0/bookmarks"));
    wod_file_t file = wod_load_file_str(strbuf_view2(bookmarks_path), ctx->framearena);
    if (wod_error()) { return; }
    strview_t data = file.view;
    while (data.size) {
        strview_t line = wstrview_get_next_line(&data);
        if (!strview_starts_with(line, cstr_SL("file:///"))) { continue; }
        strview_split_index(&line, cstr_SL("file://").size);
        File bookmark = make_file(ctx, line, &ctx->strpool_bookmarks);
        if (!bookmark.valid) { continue; }
        printfd("D: Successful read of "PRIstrw, PRIstrarg(strpool_get(bookmark.strpool, bookmark.path)));
    }
    wod_free_file(file);
}

void parse_user_dirs_dirs(Ctx *ctx) {
    // ~/.config/user-dirs.dirs
    // XDG_DESKTOP_DIR="$HOME/Desktop"
    // XDG_DOWNLOAD_DIR="$HOME/Downloads"
    // XDG_TEMPLATES_DIR="$HOME/"
    // XDG_PUBLICSHARE_DIR="$HOME/"
    // XDG_DOCUMENTS_DIR="$HOME/Documents"
    // XDG_MUSIC_DIR="$HOME/"
    // XDG_PICTURES_DIR="$HOME/Pictures"
    // XDG_VIDEOS_DIR="$HOME/Videos"
    strbuf_t* dirs_path = strbuf_create_with_arena(0, &ctx->framearena);
    strbuf_cat(&dirs_path, strbuf_view2(ctx->config), cstr_SL("/user-dirs.dirs"));
    wod_file_t file = wod_load_file_str(strbuf_view2(dirs_path), ctx->framearena);
    if (wod_error()) { return; }
    strview_t data = file.view;
    while (data.size) {
        strview_t line = wstrview_get_next_line(&data);
        printfd(PRIstrw, PRIstrarg(line));
        if (!strview_starts_with(line, cstr_SL("#"))) { continue; }
        // @Note: Not interested in implementing this yet.
    }
    wod_free_file(file);
}

void update_bookmarks(Ctx *ctx) {
    for (dyna_foreach(File, iter, ctx->bookmarks)) { free_file(iter.ref); }
    VecFile_clear_preserving(&ctx->bookmarks);
    parse_gtk3_bookmarks(ctx);
    parse_user_dirs_dirs(ctx);
    for (dyna_foreach(File, iter, ctx->bookmarks)) {
        File *file = iter.ref;
        printfd("Bookmark "PRIstrw" Alias "PRIstrw,
                PRIstrarg(strpool_get(file->strpool, file->path)),
                PRIstrarg(strpool_get(file->strpool, file->path)));
    }
}

#endif
