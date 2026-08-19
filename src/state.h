#ifndef STATE_H
#define STATE_H

#include "la_extra.h"
#include "wod_drawer.h"
#define STRPOOL_STR strview_t
#include "strpool.h"

typedef int StrpoolId;

typedef struct File {
    bool valid;
    StrpoolId path;
    bool is_dir;
    struct tm mod_date;
    StrpoolId bookmark_display_name;
    Strpool *strpool;
    //strbuf_t *path;
    //strview_t bookmark_display_name;
    //strpool_id 
    //int dir_type; // Folder or drive.
    //long edit_date;
    //long creation_date;
    //struct tm edit_date;
    //struct tm *time_info = localtime(&file_stat.st_mtime);
} File;

#define DYNA__TYPE File
#define DYNA__NAMESPACE VecFile
#include "da.h"

typedef struct Ctx {
    ArenaRoot framearena_root;
    Arena framearena;

    Strpool strpool_explorer;
    Strpool strpool_bookmarks;

    V2i window_size;

    // Assets.
    Image icon1;
    wod_font_t font1;

    // Navigation.
    VecFile bookmarks;
    VecFile history_stack;
    VecFile folder_list;
    strbuf_t *home;
    strbuf_t *config;
} Ctx;

#endif
