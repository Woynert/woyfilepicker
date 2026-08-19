#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "state.h"
#include "state_init.h"
#include "stdio.h"
#include "stdlib.h"
#include <sys/stat.h>
#include "wod_drawer.h"



void ctx_get_env_vars(Ctx *ctx) {
    const char* home = getenv("HOME");
    strview_t home_view = home ? cstr(home) : cstr_SL("/");
    strbuf_assign(&ctx->home, home_view);
    const char* config = getenv("XDG_CONFIG_HOME");
    if (config) {
        strbuf_assign(&ctx->config, cstr(config));
    } else {
        strbuf_cat(&ctx->config, strview(ctx->home), cstr_SL("/.config"));
    }
    // Make sure these don't end in '/'.
    //while (ctx->home->size > 0 && ctx->home->cstr[ctx->home->size] == '/') { --ctx->home->size; }
    //while (ctx->config->size > 0 && ctx->config->cstr[ctx->config->size] == '/') { --ctx->config->size; }
    //strbuf_assign
    //ctx->home = strview_trim_dir_separator(ctx->home);
    //ctx->config = strview_trim_dir_separator(ctx->config);
}

void parse_gtk3_bookmarks(Ctx *ctx) {
    // ~/.config/gtk-3.0/bookmarks
    int err;
    strview_t bookmarks_path = SC(&ctx->framearena, strview(ctx->config), cstr_SL("/gtk-3.0/bookmarks"));
    wod_file_t file = wod_load_file_str(bookmarks_path, ctx->framearena);
    if (wod_error()) { return; }
    strview_t data = file.view;
    while (data.size) {
        strview_t line = wstrview_get_next_line(&data);
        if (!strview_starts_with(line, cstr_SL("file:///"))) { continue; }
        strview_split_index(&line, cstr_SL("file://").size);
        strview_t linebk = line;
        strview_t bookmark_path = strview_split_first_delim(&line, " ", false);
        File bookmark; err = make_file(ctx, &bookmark, bookmark_path, &ctx->strpool_bookmarks);
        if (err != 0) { continue; }
        bool has_separator = strview_is_valid(strview_find_first(linebk, " "));
        if (has_separator) {
            strview_t alias = strview_trim_whitespace(line);
            if (!strview_is_empty(alias)) {
                File_set_alias(&bookmark, alias);
            }
        }
        VecFile_append(&ctx->bookmarks, bookmark);
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
    strbuf_cat(&dirs_path, strview(ctx->config), cstr_SL("/user-dirs.dirs"));
    wod_file_t file = wod_load_file_str(strview(dirs_path), ctx->framearena);
    if (wod_error()) { return; }
    strview_t data = file.view;
    while (data.size) {
        strview_t line = wstrview_get_next_line(&data);
        if (!strview_starts_with(line, cstr_SL("#"))) { continue; }
        //printfd(PRIstrw, PRIstrarg(line));
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
        printfd("Bookmark "PRIstrw ANSI_BLU" Alias "PRIstrw,
                PRIstrarg(File_get_path(file)),
                PRIstrarg(File_get_bookmark_alias(file)));
    }
}

#endif
