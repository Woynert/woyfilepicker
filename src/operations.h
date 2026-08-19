#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "state.h"
#include "state_init.h"
#include "stdio.h"
#include "stdlib.h"
#include <dirent.h>
#include <sys/stat.h>
#include "wod_drawer.h"

void add_location2(Ctx *ctx, const File file);

void ctx_setup(Ctx *ctx) {
    int err;
    err = make_file(ctx->framearena, &ctx->default_location, strview(ctx->home), &ctx->strpool_general);
    wassert(err == 0);
    add_location2(ctx, ctx->default_location);
    ctx_load_assets(ctx);
}


void parse_gtk_bookmarks(Ctx *ctx, strview_t bookmarks_path) {
    // ~/.config/gtk-3.0/bookmarks
    // ~/.config/gtk-4.0/bookmarks
    // Format:
    //     URI [Optional Alias]
    //     URI [Optional Alias]
    //     URI [Optional Alias]
    int err;
    wod_file_t file = wod_load_file_str(bookmarks_path, ctx->framearena);
    if (wod_error()) { return; }
    strview_t data = file.view;
    while (data.size) {
        strview_t line = wstrview_get_next_line(&data);
        if (!strview_starts_with(line, cstr_SL("file:///"))) { continue; }
        strview_split_index(&line, cstr_SL("file://").size);
        strview_t linebk = line;
        strview_t bookmark_path = strview_split_first_delim(&line, " ", false);
        File bookmark; err = make_file(ctx->framearena, &bookmark, bookmark_path, &ctx->strpool_bookmarks);
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
    parse_gtk_bookmarks(ctx, SC(&ctx->framearena, strview(ctx->config), cstr_SL("/gtk-3.0/bookmarks")));
    parse_gtk_bookmarks(ctx, SC(&ctx->framearena, strview(ctx->config), cstr_SL("/gtk-4.0/bookmarks")));
    parse_user_dirs_dirs(ctx);
    for (dyna_foreach(File, iter, ctx->bookmarks)) {
        File *file = iter.ref;
        printfd("Bookmark "PRIstrw ANSI_BLU" Alias "PRIstrw,
                PRIstrarg(File_get_path(*file)),
                PRIstrarg(File_get_bookmark_alias(*file)));
    }
}

File ctx_get_curr_dir(Ctx *ctx) {
    File *file = VecFile_get_safe(&ctx->history_stack, ctx->history_stack.size -1 -ctx->curr_location_idx);
    return file ? *file : ctx->default_location;
}

void debug_print_listing(Ctx *ctx) {
    for (dyna_foreach(File, iter, ctx->folder_list)) {
        printfd(ANSI_RED PRIstrw, PRIstrarg(File_get_path(*iter.ref)));
    }
}

void refresh_listing(Ctx *ctx) {
    // Get current path from stack.
    // @Note. No need to free files because we can just wipe the entire Strpool.
    //        So don't do: ```for file in files: free(file)```
    strpool_clear(&ctx->strpool_explorer);
    VecFile_clear_preserving(&ctx->folder_list);
    int err;
    strview_t dir_path = File_get_path(ctx_get_curr_dir(ctx));
    if (!strview_is_valid(dir_path)) { printferr("Invalid dir_path "PRIstrw, PRIstrarg(dir_path)); return; }
    strbuf_t *path_buf = strbuf_create_with_arena(dir_path, &ctx->framearena);
    if (path_buf->cstr[path_buf->size-1] != '/') { strbuf_append_cstr(&path_buf, "/"); }
    DIR *dir = opendir(path_buf->cstr);
    struct dirent *entry;
    Arena arena_bk = ctx->framearena;
    while ((entry = readdir(dir)) != NULL) {
        ctx->framearena = arena_bk;
        strview_t dname = cstr(entry->d_name);
        // Skip (. ..)
        if (dname.data[0]=='.' && (dname.size==1 || (dname.size==2 && dname.data[1] == '.'))) { continue; }
        strview_t file_path = SC(&ctx->framearena, strview(path_buf), dname);
        File file; err = make_file2(ctx->framearena, &file, file_path, &ctx->strpool_explorer);
        // Pretty fast already but can be optimized further.
        if (err != 0) { continue; }
        VecFile_append(&ctx->folder_list, file);
    }
    closedir(dir);
    ctx->framearena = arena_bk;
}

void add_location(Ctx *ctx, const strview_t arg_file_path) {
    int err;
    strview_t file_path = SC(&ctx->framearena, arg_file_path);
    File new_dir; err = make_file(ctx->framearena, &new_dir, file_path, &ctx->strpool_general);
    if (err != 0) { printferr("Invalid location ["PRIstrw"]", PRIstrarg(file_path)); return; }
    VecFile_append(&ctx->history_stack, new_dir);
}

void add_location2(Ctx *ctx, const File file) {
    int err;
    strview_t file_path = SC(&ctx->framearena, File_get_path(file));
    File new_dir; err = make_file(ctx->framearena, &new_dir, file_path, &ctx->strpool_general);
    if (err != 0) { printferr("Invalid location ["PRIstrw"]", PRIstrarg(file_path)); return; }
    VecFile_append(&ctx->history_stack, new_dir);
}

void navigate_forward(Ctx *ctx) {
    --ctx->curr_location_idx;
    ctx->curr_location_idx = int_clamp(0, ctx->history_stack.size-1, ctx->curr_location_idx);
    refresh_listing(ctx);
    debug_print_listing(ctx);
}

void navigate_backwards(Ctx *ctx) {
    ++ctx->curr_location_idx;
    ctx->curr_location_idx = int_clamp(0, ctx->history_stack.size-1, ctx->curr_location_idx);
    refresh_listing(ctx);
    debug_print_listing(ctx);
}

void navigate_parent_dir(Ctx *ctx) {
    // Try get parent.
    File curr_dir = ctx_get_curr_dir(ctx);
    strview_t dir_path = File_get_path_copy(curr_dir, &ctx->framearena);
    if (dir_path.size <= 1) { return; }
    dir_path = strview_trim_dir_separator(dir_path);
    printfd("We are in "PRIstrw, PRIstrarg(dir_path));
    //strview_t parent_path = dir_path;
    strview_t dir_path2 = dir_path;
    strview_t parent_path = strview_split_left(&dir_path2, strview_split_last_delim(&dir_path, "/", false));
    printfd("Parent is "PRIstrw, PRIstrarg(parent_path));
    add_location(ctx, parent_path);
    refresh_listing(ctx);
    debug_print_listing(ctx);
}

#endif
