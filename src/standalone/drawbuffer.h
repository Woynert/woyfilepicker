// [NOT PART OF HEADER]
#include "state.h"
#define DBUF_IMG_T  Image
#define DBUF_FONT_T wod_font_t
#define DBUF__DEBUG
// [!NOT PART OF HEADER]

/*
   Simple draw buffer for 2D raylib commands:

   Usage:
   You must define these types depending on your project.

    #define DBUF_IMG_T  MyImageType
    #define DBUF_FONT_T MyFontType
    #include "raylib_drawbuffer.h"
   */

#ifndef DRAWBUFFER_H
#define DRAWBUFFER_H

#if (!defined(DBUF_IMG_T) || !defined(DBUF_FONT_T))
    #error "Must define Image type and Font type."
    #define DBUF_IMG_T  int
    #define DBUF_FONT_T int
#endif

#include <stdlib.h>
#include "arenady.h"
#include "la_extra.h"
#include "wstrview.h"
#include "raylib_lite.h"
#include "portable_utils.h"

typedef enum {
    DRAWCMD_RECT,
    DRAWCMD_FRAME,
    DRAWCMD_TEXT,
    DRAWCMD_TEXTURE,
    DRAWCMD_BEGIN_SCISSOR,
    DRAWCMD_END_SCISSOR,
} DrawCmd;

typedef struct {
    #define STRUCT_MEMBERS       \
    X( Rect2i , r        )       \
    X( Color  , color    )
    #define X(type, name) type name;
    STRUCT_MEMBERS
    #undef X
} dbuf_draw_rect_args_t;
#define STRUCT_NAME dbuf_draw_rect_args_t
#include "struct_assert_no_padding.h"

typedef struct {
    #define STRUCT_MEMBERS       \
    X( Rect2i , r        )       \
    X( Color  , color    )       \
    X( int    , thickness)
    #define X(type, name) type name;
    STRUCT_MEMBERS
    #undef X
} dbuf_draw_frame_args_t;
#define STRUCT_NAME dbuf_draw_frame_args_t
#include "struct_assert_no_padding.h"

typedef struct {
    #define STRUCT_MEMBERS           \
    X( DBUF_FONT_T, font           ) \
    X( V2i      , position         ) \
    X( int      , font_size        ) \
    X( int      , spacing          ) \
    X( int      , textLineSpacing  ) \
    X( Color    , tint             ) \
    X( int      , __pad            ) \
    X( int      , str_size         )
    #define X(type, name) type name;
    STRUCT_MEMBERS
    #undef X
    char str_data[];
} dbuf_draw_text_args_t;
#define STRUCT_NAME dbuf_draw_text_args_t
#include "struct_assert_no_padding.h"

typedef struct {
    #define STRUCT_MEMBERS  \
    X( Rect2i , r       )
    #define X(type, name) type name;
    STRUCT_MEMBERS
    #undef X
} dbuf_scissor_args_t;
#define STRUCT_NAME dbuf_scissor_args_t
#include "struct_assert_no_padding.h"

typedef struct {
    #define STRUCT_MEMBERS      \
    X( DBUF_IMG_T , img       ) \
    X( Rect2i     , source    ) \
    X( Rect2i     , dest      ) \
    X( V2i        , origin    ) \
    X( float      , rotation  ) \
    X( Color      , tint      )
    #define X(type, name) type name;
    STRUCT_MEMBERS
    #undef X
} dbuf_draw_texture_args_t;
#define STRUCT_NAME dbuf_draw_texture_args_t
#include "struct_assert_no_padding.h"


typedef void (*dbuf_draw_rect_t)  (Rect2i rect, Color color);
typedef void (*dbuf_draw_frame_t) (Rect2i rect, Color color, int thickness);
typedef void (*dbuf_draw_text_t)  (
        strview_t str,
        DBUF_FONT_T font,
        V2i position,
        int font_size,
        int spacing,
        int textLineSpacing,
        Color color);
typedef void (*dbuf_draw_image_t) (
        DBUF_IMG_T img,
        Rect2i source,
        Rect2i dest,
        V2i origin,
        float rotation,
        Color tint);
typedef void (*dbuf_scissor_t)    (bool start_end, Rect2i rect);

#define DYNA__TYPE DrawCmd
#define DYNA__NAMESPACE Vec_DrawCmd
#include "da.h"

typedef struct dbuf_Layer {
    ArenaDy arena;
    Vec_DrawCmd commands;
} dbuf_layer_t;

typedef struct {
    dbuf_layer_t layers[256];
} dbuf_buffer_t;

struct {
    dbuf_buffer_t swap[2];
    dbuf_buffer_t *curr;
    uint8_t currlayer;

    dbuf_draw_rect_t  draw_rect_cb;
    dbuf_draw_frame_t draw_frame_cb;
    dbuf_draw_text_t  draw_text_cb;
    dbuf_draw_image_t draw_image_cb;
    dbuf_scissor_t    scissor_cb;
} dbuf__ctx = { 0 };

void dbuf_init(void) {
    for (int k = 0; k < countofi(dbuf__ctx.swap); ++k) {
        dbuf__ctx.curr = &dbuf__ctx.swap[k];
        for (int i = 0; i < countofi(dbuf__ctx.curr->layers); ++i) {
            dbuf_layer_t *layer = &dbuf__ctx.curr->layers[i];
            layer->arena = arenady_create();
            layer->commands = Vec_DrawCmd_create();
        }
    }
}

void dbuf_setup_callbacks(
    dbuf_draw_rect_t  draw_rect_cb,
    dbuf_draw_frame_t draw_frame_cb,
    dbuf_draw_text_t  draw_text_cb,
    dbuf_draw_image_t draw_image_cb,
    dbuf_scissor_t    scissor_cb
) {
    wassert(draw_rect_cb);
    wassert(draw_frame_cb);
    wassert(draw_text_cb);
    wassert(draw_image_cb);
    wassert(scissor_cb);
    dbuf__ctx.draw_rect_cb  = draw_rect_cb;
    dbuf__ctx.draw_frame_cb = draw_frame_cb;
    dbuf__ctx.draw_text_cb  = draw_text_cb;
    dbuf__ctx.draw_image_cb = draw_image_cb;
    dbuf__ctx.scissor_cb    = scissor_cb;
}

void dbuf_deinit(void) {
    for (int k = 0; k < countofi(dbuf__ctx.swap); ++k) {
        dbuf__ctx.curr = &dbuf__ctx.swap[k];
        for (int i = 0; i < countofi(dbuf__ctx.curr->layers); ++i) {
            dbuf_layer_t *layer = &dbuf__ctx.curr->layers[i];
            arenady_free(&layer->arena);
            Vec_DrawCmd_free(&layer->commands);
        }
    }
}

void dbuf_set_layer(uint8_t layer) {
    dbuf__ctx.currlayer = layer;
}

uint8_t dbuf_get_layer(void) { return dbuf__ctx.currlayer; }

void dbuf__print_binary(const void *data, size_t length) {
    const unsigned char *byte_ptr = (const unsigned char *)data;
    printf(ANSI_RED"HEX DATA\n");
    for (size_t i = 0; i < length; i++) { printf("%02X ", byte_ptr[i]); }
    printf("\n");
}

bool dbuf_do_buffers_differ(void) {
    for (int i = 0; i < countofi(dbuf__ctx.swap[0].layers); ++i) {
        dbuf_layer_t *la = &dbuf__ctx.swap[0].layers[i];
        dbuf_layer_t *lb = &dbuf__ctx.swap[1].layers[i];
#ifdef DBUF__DEBUG
        if ((la->commands.size != lb->commands.size)) {
            printfd(ANSI_GRE"(layer%d) Buffers differ 1 (%d, %d)", i, la->commands.size, lb->commands.size);
            return true;
        }
        if ((la->arena.beg - la->arena.root) != (lb->arena.beg - lb->arena.root)) {
            printfd(ANSI_GRE"(layer%d) Buffers differ 2 (%ld, %ld)", i, (la->arena.beg - la->arena.root), (lb->arena.beg - lb->arena.root));
            dbuf__print_binary(la->arena.root, (size_t)(la->arena.beg - la->arena.root));
            dbuf__print_binary(lb->arena.root, (size_t)(lb->arena.beg - lb->arena.root));
            return true;
        }
        if ((la->commands.size != 0 && memcmp(la->commands.items, lb->commands.items, (size_t)la->commands.size * sizeof(la->commands.items[0])))) {
            printfd(ANSI_GRE"(layer%d) Buffers differ 3", i);
            return true;
        }
        if (memcmp(la->arena.root, lb->arena.root, (size_t)(la->arena.beg - la->arena.root))) {
            printfd(ANSI_GRE"(layer%d) Buffers differ 4", i);
            dbuf__print_binary(la->arena.root, (size_t)(la->arena.beg - la->arena.root));
            dbuf__print_binary(lb->arena.root, (size_t)(la->arena.beg - la->arena.root));
            return true;
        }
#endif
        if (
           (la->commands.size != lb->commands.size)
           || ((la->arena.beg - la->arena.root) != (lb->arena.beg - lb->arena.root))
           || (la->commands.size != 0 && memcmp(la->commands.items, lb->commands.items, (size_t)la->commands.size * sizeof(la->commands.items[0])))
           || (memcmp(la->arena.root, lb->arena.root, (size_t)(la->arena.beg - la->arena.root)))
        ) {
            return true;
        }
    }
    return false;
}

void dbuf_draw_start(void) {
    // Swap.
    dbuf__ctx.curr = dbuf__ctx.curr == &dbuf__ctx.swap[0] ? &dbuf__ctx.swap[1] : &dbuf__ctx.swap[0];
    for (int i = 0; i < countofi(dbuf__ctx.curr->layers); ++i) {
        dbuf_layer_t *layer = &dbuf__ctx.curr->layers[i];
        arenady_reset_beginning(&layer->arena);
        Vec_DrawCmd_clear_preserving(&layer->commands);
    }
}

void dbuf_draw_end(void) {
    for (int i = 0; i < countofi(dbuf__ctx.curr->layers); ++i) {
        dbuf_layer_t *layer = &dbuf__ctx.curr->layers[i];
        arenady_reset_beginning(&layer->arena);
        for (dyna_foreach(DrawCmd, iter, layer->commands)) {
            DrawCmd cmd = *iter.ref;
            switch (cmd) {
                case DRAWCMD_RECT:
                {
                    dbuf_draw_rect_args_t *args = arenady_new(&layer->arena, dbuf_draw_rect_args_t, 1);
                    dbuf__ctx.draw_rect_cb(args->r, args->color);
                    printfd("DRAWCMD_RECT");
                    dbuf__print_binary(args, sizeof(dbuf_draw_rect_args_t));
                } break;
                case DRAWCMD_FRAME:
                {
                    dbuf_draw_frame_args_t *args = arenady_new(&layer->arena, dbuf_draw_frame_args_t, 1);
                    dbuf__ctx.draw_frame_cb(args->r, args->color, args->thickness);
                    printfd("DRAWCMD_FRAME");
                    dbuf__print_binary(args, sizeof(dbuf_draw_frame_args_t));
                } break;
                case DRAWCMD_TEXT:
                {
                    dbuf_draw_text_args_t *args = arenady_new(&layer->arena, dbuf_draw_text_args_t, 1);
                    arenady_new(&layer->arena, char, args->str_size);
                    strview_t str = { .data = args->str_data, .size = args->str_size };
                    dbuf__ctx.draw_text_cb(str, args->font, args->position, args->font_size, args->spacing, args->textLineSpacing, args->tint);
                } break;
                case DRAWCMD_TEXTURE:
                {
                    dbuf_draw_texture_args_t *args = arenady_new(&layer->arena, dbuf_draw_texture_args_t, 1);
                    dbuf__ctx.draw_image_cb(args->img, args->source, args->dest, args->origin, args->rotation, args->tint);
                } break;
                case DRAWCMD_BEGIN_SCISSOR:
                {
                    dbuf_scissor_args_t *args = arenady_new(&layer->arena, dbuf_scissor_args_t, 1);
                    dbuf__ctx.scissor_cb(true, args->r);
                } break;
                case DRAWCMD_END_SCISSOR:
                {
                    dbuf__ctx.scissor_cb(false, (Rect2i){0});
                } break;
            }
        }
    }
}

void b_draw_texture_ext(DBUF_IMG_T img, Rect2i source, Rect2i dest, V2i origin, float rotation, Color tint) {
    dbuf_layer_t *layer = &dbuf__ctx.curr->layers[dbuf__ctx.currlayer];
    dbuf_draw_texture_args_t *args = arenady_new(&layer->arena, dbuf_draw_texture_args_t, 1);
    // @Note(woy): Zeroing user provided type.
    memset(&args->img, 0, sizeof(args->img));
    *args = (dbuf_draw_texture_args_t) { img, source, dest, origin, rotation, tint };
    Vec_DrawCmd_append(&layer->commands, DRAWCMD_TEXTURE);
}

void b_draw_rect(Rect2i r, Color color) {
    dbuf_layer_t *layer = &dbuf__ctx.curr->layers[dbuf__ctx.currlayer];
    dbuf_draw_rect_args_t *args = arenady_new(&layer->arena, dbuf_draw_rect_args_t, 1);
    *args = (dbuf_draw_rect_args_t) { 0 };
    args->r = r;
    args->color = color;
    Vec_DrawCmd_append(&layer->commands, DRAWCMD_RECT);
}

void b_draw_frame(Rect2i r, Color color, int thickness) {
    dbuf_layer_t *layer = &dbuf__ctx.curr->layers[dbuf__ctx.currlayer];
    dbuf_draw_frame_args_t *args = arenady_new(&layer->arena, dbuf_draw_frame_args_t, 1);
    args->r = r;
    args->color = color;
    args->thickness = thickness;
    Vec_DrawCmd_append(&layer->commands, DRAWCMD_FRAME);
}

void b_draw_text_ext(DBUF_FONT_T font, const strview_t str, V2i pos, int font_size, int spacing, int textLineSpacing, Color tint) {
    if (str.size == 0 || str.data == NULL) { return; }
    dbuf_layer_t *layer = &dbuf__ctx.curr->layers[dbuf__ctx.currlayer];
    // Allocate struct + string in one call.
    dbuf_draw_text_args_t *args = (dbuf_draw_text_args_t *)
        arenady_alloc(&layer->arena, (i64)(sizeof(dbuf_draw_text_args_t) + (size_t)str.size), _Alignof(dbuf_draw_text_args_t), 1);
    // @Note(woy): Zeroing user provided type.
    memset(&args->font, 0, sizeof(args->font));
    *args = (dbuf_draw_text_args_t) {
        .font = font, .position = pos, .font_size = font_size, .spacing = spacing,
        .textLineSpacing = textLineSpacing, .tint = tint, .str_size = str.size,
    };
    memmove(args->str_data, str.data, (size_t)str.size);
    Vec_DrawCmd_append(&layer->commands, DRAWCMD_TEXT);
}

void b_draw_begin_scissor(Rect2i r) {
    dbuf_layer_t *layer = &dbuf__ctx.curr->layers[dbuf__ctx.currlayer];
    dbuf_scissor_args_t *args = arenady_new(&layer->arena, dbuf_scissor_args_t, 1);
    args->r = r;
    Vec_DrawCmd_append(&layer->commands, DRAWCMD_BEGIN_SCISSOR);
}

void b_draw_end_scissor(void) {
    dbuf_layer_t *layer = &dbuf__ctx.curr->layers[dbuf__ctx.currlayer];
    Vec_DrawCmd_append(&layer->commands, DRAWCMD_END_SCISSOR);
}

// Extra functions for easy of use.

void b_draw_text(DBUF_FONT_T font, const strview_t str, V2i pos, int font_size, Color tint) {
    b_draw_text_ext(font, str, pos, font_size, 0, 0, tint);
}

void b_draw_texture(DBUF_IMG_T img, Rect2i dest) {
    b_draw_texture_ext(img, (Rect2i){{0,0,img.size.x,img.size.y}}, dest, v2ii(0), 0, WHITE);
}

#endif
