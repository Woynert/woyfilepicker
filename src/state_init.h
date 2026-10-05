#ifndef STATE_INIT_H
#define STATE_INIT_H

#include "state.h"
#include "textbox_visual.h"
#include "stbtt_extra.h"
#include "wod_drawer.h"
#include <sys/stat.h>

int make_file(Arena scratch, File *out_file, strview_t path, Strpool *strpool);

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
    // NOTE: We could definitely do some sanitizing on HOME and config.
}

void ctx_init(Ctx *ctx) {
    *ctx = (Ctx){0};
    ctx->framearena_root = ArenaRoot_create(1024 * 1024);
    ctx->framearena = ArenaRoot_get_arena(ctx->framearena_root);
    strpool_create(&ctx->strpool_explorer);
    strpool_create(&ctx->strpool_bookmarks);
    strpool_create(&ctx->strpool_general);
    ctx->home = strbuf_create(0, NULL);
    ctx->config = strbuf_create(0, NULL);
    ctx->bookmarks = VecFile_create();
    ctx->history_stack = VecFile_create();
    ctx->folder_files = VecFile_create();
    Focuser_create(&ctx->focuser);
    mapstrobj_create(&ctx->frame_objs);

    ctx_get_env_vars(ctx);
}

void ctx_load_assets(Ctx *ctx) {
    //ctx->icon1 = load_image(cstr_SL("assets/imgdemox64.png"));
    ctx->icon1 = load_image(cstr_SL("assets/imgdemo3.png")); wassert(!wod_error());
    ctx->icon_folder = load_image(cstr_SL("assets/icon_folder.png")); wassert(!wod_error());
    ctx->icon_file = load_image(cstr_SL("assets/icon_file.png")); wassert(!wod_error());
    ctx->icon_up = load_image(cstr_SL("assets/icon_arrow_up.png")); wassert(!wod_error());
    ctx->icon_left = load_image(cstr_SL("assets/icon_arrow_left.png")); wassert(!wod_error());
    ctx->icon_right = load_image(cstr_SL("assets/icon_arrow_right.png")); wassert(!wod_error());
    ctx->icon_search = load_image(cstr_SL("assets/icon_magniglass.png")); wassert(!wod_error());

    wod_file_t file = wod_load_file("./assets/Roboto-Regular.ttf");
    wassert(!wod_error());
    wod_free_file(file);

   const V2i ranges[] = { // Ranges are inclusive
      {{ 0xFFFD,  0xFFFD }},  // (�) codepoint
      {{ 32,      127 }},     // Basic latin
      {{ 0x00A1,  0x00FF }},  // C1 Controls and Latin-1 Supplement
      {{ 0x0100,  0x017F }},  // Latin Extended-A
      {{ 0x0180,  0x024F }},  // Latin Extended-B
      {{ 0x1F300, 0x1F5FF }}, // Miscellaneous Symbols and Pictographs
      {{ 0x1F600, 0x1F64F }}, // Emoticons
   };

    wod_font_t font = load_font(ctx->framearena, cstr_SL("./assets/Roboto-Regular.ttf"), 18, (V2i*)ranges, countofi(ranges));
    wassert(!wod_error());
    //stbtt_print_bitmap((unsigned char*)font.bitmap.data, font.bitmap.size.x, font.bitmap.size.y);
    ctx->font1 = font;

    font = load_font(ctx->framearena, cstr_SL("./assets/IosevkaFixed-Regular.ttf"), 18, (V2i*)ranges, countofi(ranges));
    stbtt_print_bitmap((unsigned char*)font.bitmap.data, font.bitmap.size.x, font.bitmap.size.y);
    wassert(!wod_error());
    ctx->font_mono = font;

    TextboxVisual_setup(&ctx->tbox_path_visual, ctx->font1, 2, 0);
    TextboxVisual_setup(&ctx->tbox_search_visual, ctx->font1, 2, 0);
}


void ctx_free(Ctx *ctx) {
    ArenaRoot_free(&ctx->framearena_root);
    strpool_destroy(&ctx->strpool_bookmarks);
    strpool_destroy(&ctx->strpool_explorer);
    strpool_destroy(&ctx->strpool_general);
    strbuf_destroy(&ctx->home);
    strbuf_destroy(&ctx->config);
    VecFile_free(&ctx->bookmarks);
    VecFile_free(&ctx->history_stack);
    VecFile_free(&ctx->folder_files);
    textbox_free(&ctx->tbox_path);
    textbox_free(&ctx->tbox_search);
    Focuser_free(&ctx->focuser);
    mapstrobj_free(&ctx->frame_objs);

    free_font(ctx->font1);
    free_font(ctx->font_mono);
    free_image(ctx->icon1);
    free_image(ctx->icon_folder);
    free_image(ctx->icon_file);
    free_image(ctx->icon_up);
    free_image(ctx->icon_left);
    free_image(ctx->icon_right);
    free_image(ctx->icon_search);
}

void free_file(File *file) {
    strpool_remove(file->strpool, file->path);
    strpool_remove(file->strpool, file->bookmark_alias);
    *file = (File) { 0 };
}

void File_set_alias(File *file, strview_t alias) {
    if (!ID_equals(file->path, file->bookmark_alias)) {
        // Clean up old alias.
        strview_t prev_alias = strpool_get(file->strpool, file->bookmark_alias);
        if (strview_is_valid(prev_alias)) {
            wassert(0 == strpool_remove(file->strpool, file->bookmark_alias));
            file->bookmark_alias = ID_INVALID;
        }
    }
    alias = strview_trim_whitespace(alias);
    if (!strview_is_empty(alias)) {
        file->bookmark_alias = strpool_append(file->strpool, alias);
    } else {
        file->bookmark_alias = file->path;
    }
}

/// @Note: No need to call free_file on failure.
/// @Returns error.
int make_file(Arena scratch, File *out_file, strview_t path, Strpool *strpool) {
    File file = { .strpool = strpool };
    strbuf_t *path_buf = strbuf_create_with_arena(path, &scratch);
    {
        struct stat path_stat;
        if (stat(path_buf->cstr, &path_stat) != 0) { return -1; }
        struct tm *mod_date = localtime(&path_stat.st_mtime);
        file.mod_date_secs_epoc = path_stat.st_mtime;
        file.mod_date = *mod_date;
        file.is_dir = path_stat.st_mode & __S_IFDIR;
    }
    file.path = strpool_append(strpool, path);
    path = strview_trim_dir_separator(path);
    strview_t alias = strview_split_last_delim(&path, "/", false);
    File_set_alias(&file, alias);
    *out_file = file;
    return 0;
}

int make_file2(Arena scratch, File *out_file, strview_t path, Strpool *strpool) {
    File file = { .strpool = strpool };
    strbuf_t *path_buf = strbuf_create_with_arena(path, &scratch);
    {
        struct stat path_stat;
        if (stat(path_buf->cstr, &path_stat) != 0) { return -1; }
        struct tm *mod_date = localtime(&path_stat.st_mtime);
        file.mod_date_secs_epoc = path_stat.st_mtime;
        file.mod_date = *mod_date;
        file.is_dir = path_stat.st_mode & __S_IFDIR;
    }
    file.path = strpool_append(strpool, path);
    strview_t alias = strview_split_last_delim(&path, "/", false);
    file.bookmark_alias = strpool_append(file.strpool, alias);
    *out_file = file;
    return 0;
}

/*
int make_file2(Ctx *ctx, File *out_file, strview_t path, Strpool *strpool, bool is_dir) {
    File file = { .strpool = strpool };
    strbuf_t *path_buf = strbuf_create_with_arena(path, &ctx->framearena);
    {
        struct stat path_stat;
        if (stat(path_buf->cstr, &path_stat) != 0) { return -1; }
        struct tm *mod_date = localtime(&path_stat.st_mtime);
        file.mod_date = *mod_date;
        file.is_dir = path_stat.st_mode & __S_IFDIR;
    }
    file.path = strpool_append(strpool, path);
    path = strview_trim_dir_separator(path);
    strview_t alias = strview_split_last_delim(&path, "/", false);
    File_set_alias(&file, alias);
    *out_file = file;
    return 0;
}
*/

strview_t File_get_path_copy(const File file, Arena *perm) {
    return SC(perm, strpool_get(file.strpool, file.path));
}

strview_t File_get_path(const File file) {
    return strpool_get(file.strpool, file.path);
}

strview_t File_get_bookmark_alias(const File file) {
    strview_t alias = strpool_get(file.strpool, file.bookmark_alias);
    if (!strview_is_empty(alias)) return alias;
    return strpool_get(file.strpool, file.path);
}

#endif // !STATE_INIT_H
