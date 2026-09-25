#ifndef STATE_H
#define STATE_H

#include "la_extra.h"
#define TEXTBOX_VISUAL__ONLY_HEADER
#include "textbox_visual.h"
#include "wod_drawer.h"
#define STRPOOL_STR strview_t
#include "strpool.h"

bool MUST_CLOSE = false;
bool MUST_REDRAW = false;
bool MUST_RESIZE = false;
bool FORCE_UI_REDRAW = false;

typedef int StrpoolId;

typedef struct File {
    ID path;
    bool is_dir;
    struct tm mod_date;
    long mod_date_secs_epoc;
    ID bookmark_alias;
    Strpool *strpool;
} File;

#define DYNA__TYPE File
#define DYNA__NAMESPACE VecFile
#include "da.h"

typedef enum {
    FILE_SORT_NAME,
    FILE_SORT_DATE,
} FileSort;

typedef struct Ctx {
    ArenaRoot framearena_root;
    Arena framearena;

    Strpool strpool_explorer;
    Strpool strpool_bookmarks;
    Strpool strpool_general;

    V2i window_size;

    Textbox tbox_path;
    TextboxVisual tbox_path_visual;
    Textbox tbox_search;
    TextboxVisual tbox_search_visual;

    // Navigation.
    strbuf_t *home;
    strbuf_t *config;
    File default_location;
    int curr_location_cursor;
    VecFile bookmarks;
    VecFile history_stack;
    VecFile folder_files;
    int first_file_idx; // First file in ctx.folder_files.

    // Explorer display
    FileSort file_sort;
    bool inverted;

    // Assets.
    struct {
        wod_font_t font1;
        wod_font_t font_mono;
        Image icon1;
        Image icon_folder;
        Image icon_file;
        Image icon_up;
        Image icon_left;
        Image icon_right;
        Image icon_search;
    };
} Ctx;

#endif
