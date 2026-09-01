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
    // The format is as follows:
    // URI [Optional Alias]
    // URI [Optional Alias]
    // URI [Optional Alias]
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
    {
        // Add user home.
        int err;
        File home; err = make_file(ctx->framearena, &home, strview(ctx->home), &ctx->strpool_bookmarks);
        if (!err) { VecFile_append(&ctx->bookmarks, home); }
        // Add file system root.
        File bookmark_root; err = make_file(ctx->framearena, &bookmark_root, cstr_SL("/"), &ctx->strpool_bookmarks);
        if (!err) {
            File_set_alias(&bookmark_root, cstr_SL("File System"));
            VecFile_append(&ctx->bookmarks, bookmark_root);
        }
    }
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
    File *file = VecFile_get_safe(&ctx->history_stack, ctx->history_stack.size -1 -ctx->curr_location_cursor);
    return file ? *file : ctx->default_location;
}

int ctx_get_history_stack_idx(Ctx *ctx, int offset) {
    return ctx->history_stack.size -1 -offset;
}


void debug_print_history_stack(Ctx *ctx) {
    printfd("[ ↓ HISTORY STACK]");
    for (dyna_foreach(File, iter, ctx->history_stack)) {
        printfd(ANSI_RED PRIstrw "%s", PRIstrarg(File_get_path(*iter.ref)),
            iter.index == ctx_get_history_stack_idx(ctx, ctx->curr_location_cursor) ? " <--" : ""
        );
    }
    printfd("[ ↑ HISTORY STACK]");
}

void debug_print_listing(Ctx *ctx) {
    strview_t dir_path = File_get_path_copy(ctx_get_curr_dir(ctx), &ctx->framearena);
    printfd("YOU ARE IN ["PRIstrw"]", PRIstrarg(dir_path));
    for (dyna_foreach(File, iter, ctx->folder_files)) {
        printfd(ANSI_RED PRIstrw, PRIstrarg(File_get_path(*iter.ref)));
    }
    printfd("YOU ARE IN ["PRIstrw"]", PRIstrarg(dir_path));
    debug_print_history_stack(ctx);
}

void set_file_sort(Ctx *ctx, FileSort sort) {
    ctx->file_sort = sort;
    ctx->inverted = false;
}

void toggle_invert_file_sort(Ctx *ctx) {
    ctx->inverted = !ctx->inverted;
}


int file_compare_by_dir(const void *p1, const void *p2) {
    (void)p1;
    const File *file2 = (File*)p2;
    return file2->is_dir;
}

int file_compare_by_date(const void *p1, const void *p2) {
    const File *file1 = (File*)p1;
    const File *file2 = (File*)p2;
    return file1->mod_date_secs_epoc > file2->mod_date_secs_epoc;
}

int file_compare_by_name(const void *p1, const void *p2) {
    const File *file1 = (File*)p1;
    const File *file2 = (File*)p2;
    strview_t alias1 = File_get_bookmark_alias(*file1);
    strview_t alias2 = File_get_bookmark_alias(*file2);
    return strncasecmp(alias1.data, alias2.data, (size_t)int_min(alias1.size, alias2.size));
}

void generate_sorted_display_file_list(Ctx *ctx) {
    typedef int (*qsort_compare_func_t) (const void *, const void *);
    qsort_compare_func_t compare_fun = file_compare_by_name;
    switch (ctx->file_sort) {
        case FILE_SORT_NAME: { compare_fun = file_compare_by_name; break; }
        case FILE_SORT_DATE: { compare_fun = file_compare_by_date; break; }
        default: wassert(false);
    }

    ctx->first_file_idx = int_clamp(0, ctx->folder_files.size, ctx->first_file_idx);
    qsort(ctx->folder_files.items,
            (size_t)ctx->first_file_idx,
            sizeof(ctx->folder_files.items[0]),
            compare_fun);
    qsort(ctx->folder_files.items + ctx->first_file_idx,
            (size_t)(ctx->folder_files.size - ctx->first_file_idx),
            sizeof(ctx->folder_files.items[0]),
            compare_fun);
}

void refresh_listing(Ctx *ctx) {
    // @Note. No need to free files because we can just wipe the entire Strpool.
    //        So don't do: ```for file in files: free(file)```
    strpool_clear(&ctx->strpool_explorer);
    VecFile_clear_preserving(&ctx->folder_files);
    // Get current path from stack.
    int err;
    strview_t dir_path = File_get_path(ctx_get_curr_dir(ctx));
    if (!strview_is_valid(dir_path)) { printferr("Invalid dir_path "PRIstrw, PRIstrarg(dir_path)); return; }
    strbuf_t *path_buf = strbuf_create_with_arena(dir_path, &ctx->framearena);
    if (path_buf->cstr[path_buf->size-1] != '/') { strbuf_append_cstr(&path_buf, "/"); }
    DIR *dir = opendir(path_buf->cstr);
    if (!dir) { printferr("Can't opendir [%s]", path_buf->cstr); return; }
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
        VecFile_append(&ctx->folder_files, file);
    }
    closedir(dir);
    ctx->framearena = arena_bk;

    // Sort so that folders appear at the top.
    qsort(ctx->folder_files.items,
            (size_t)ctx->folder_files.size,
            sizeof(ctx->folder_files.items[0]),
            file_compare_by_dir);

    // Find separator where folders end.
    for(dyna_foreach(File, iter, ctx->folder_files)) {
        if (!iter.ref->is_dir) { ctx->first_file_idx = iter.index; break; }
    }

    set_file_sort(ctx, FILE_SORT_NAME);
    generate_sorted_display_file_list(ctx); // MOVEME.
}

void invert_array_items_in_range_inclusive(File *items, int from, int to) {
    for (int i = from, k = to; i < from + (to+1-from)/2; ++i, --k) {
        File bk = items[i];
        items[i] = items[k];
        items[k] = bk;
    }
}

void add_location(Ctx *ctx, const strview_t arg_file_path) {
    int err;
    strview_t file_path = SC(&ctx->framearena, arg_file_path);
    File location; err = make_file(ctx->framearena, &location, file_path, &ctx->strpool_general);
    if (err != 0) {
        printferr("Invalid location ["PRIstrw"]", PRIstrarg(file_path)); return;
    }
    if (!location.is_dir) {
        printferr("Invalid location ["PRIstrw"]. Not a directory.", PRIstrarg(file_path));
        free_file(&location);
        return;
    }
    printfd("Navigating to file ["PRIstrw"]", PRIstrarg(File_get_path(location)));

    if (ctx->curr_location_cursor != 0) { do {
        int curr_idx = ctx_get_history_stack_idx(ctx, ctx->curr_location_cursor);
        invert_array_items_in_range_inclusive(ctx->history_stack.items, curr_idx+1, ctx->history_stack.size-1);
        File current;
        err = VecFile_pop_at_preserve_order(&ctx->history_stack, curr_idx, &current);
        if (err != 0) { break; }
        VecFile_append(&ctx->history_stack, current);
    } while(0); }
    VecFile_append(&ctx->history_stack, location);
    ctx->curr_location_cursor = 0;

    // Remove contiguous duplicates.
    
    strview_t path = STRVIEW_INVALID;
    for (dyna_foreach_reverse(File, iter, ctx->history_stack)) {
        file_path = File_get_path_copy(*iter.ref, &ctx->framearena);
        if (strview_equal(path, file_path)) {
            File out;
            err = VecFile_pop_at_preserve_order(&ctx->history_stack, iter.index, &out);
            if (err == 0) {
                free_file(&out);
            }
        } else {
            path = file_path;
        }
    }
}

void add_location2(Ctx *ctx, const File file) {
    add_location(ctx, File_get_path(file));
}

bool can_navigate_forward(Ctx *ctx) { return ctx->curr_location_cursor > 0; }
bool can_navigate_backwards(Ctx *ctx) { return ctx->curr_location_cursor < ctx->history_stack.size-1; }

void navigate_forward(Ctx *ctx) {
    --ctx->curr_location_cursor;
    ctx->curr_location_cursor = int_clamp(0, ctx->history_stack.size-1, ctx->curr_location_cursor);
    refresh_listing(ctx);
    //debug_print_listing(ctx);
}

void navigate_backwards(Ctx *ctx) {
    ++ctx->curr_location_cursor;
    ctx->curr_location_cursor = int_clamp(0, ctx->history_stack.size-1, ctx->curr_location_cursor);
    refresh_listing(ctx);
    //debug_print_listing(ctx);
}

void navigate_parent_dir(Ctx *ctx) {
    // Try get parent.
    strview_t dir_path = File_get_path_copy(ctx_get_curr_dir(ctx), &ctx->framearena);
    if (dir_path.size <= 1) { return; }
    dir_path = strview_trim_dir_separator(dir_path);
    printfd("We are in "PRIstrw, PRIstrarg(dir_path));
    //strview_t parent_path = dir_path;
    strview_t dir_path2 = dir_path;
    strview_t parent_path = strview_split_left(&dir_path2, strview_split_last_delim(&dir_path, "/", false));
    printfd("Parent is "PRIstrw, PRIstrarg(parent_path));
    add_location(ctx, parent_path);
    refresh_listing(ctx);
    //debug_print_listing(ctx);
}

#endif
