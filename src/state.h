#ifndef STATE_H
#define STATE_H

#include "la_extra.h"
#include "wod_drawer.h"
#include "textbox.h"
#define STRPOOL_STR strview_t
#include "strpool.h"

bool MUST_CLOSE = false;
bool MUST_REDRAW = false;
bool MUST_RESIZE = false;
bool FORCE_UI_REDRAW = false;

typedef int StrpoolId;

typedef struct File {
    StrpoolId path;
    bool is_dir;
    struct tm mod_date;
    StrpoolId bookmark_alias;
    Strpool *strpool;
} File;

#define DYNA__TYPE File
#define DYNA__NAMESPACE VecFile
#include "da.h"

typedef struct Ctx {
    ArenaRoot framearena_root;
    Arena framearena;

    Strpool strpool_explorer;
    Strpool strpool_bookmarks;
    Strpool strpool_general;

    V2i window_size;
    Textbox tbox_path;
    Textbox tbox_search;

    // Assets.
    Image icon1;
    wod_font_t font1;
    wod_font_t font_mono;

    // Navigation.
    VecFile bookmarks;
    VecFile history_stack;
    VecFile folder_list;
    strbuf_t *home;
    strbuf_t *config;
    File default_location;
    int curr_location_cursor;
} Ctx;

#endif
