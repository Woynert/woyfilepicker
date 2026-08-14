#ifndef WOD_H
#define WOD_H

#include "stb_image.h"
#include "arena.h"
#include "arena_extra.h"
#include "strview.h"
#include "wstrview.h"
#include "strbuf_extra.h"
#include "strbuf.h"
#include "portable_utils.h"
#include "la_extra.h"
#include "raylib_lite.h"
#include <stdint.h>

#define STB_RGBA 4

typedef uint8_t  u8;
typedef int32_t  i32;
typedef uint32_t u32;
typedef float    f32;

typedef struct {
    V2i size;
    union {
        char *data;
        u32 *pixels; // Always rgba.
    };
} Image;

struct {
    int error;
    ArenaRoot arenaroot;
} wod__ctx = { 0 };


Arena wod__get_arena(void) {
    if (wod__ctx.arenaroot.buf == NULL) {
        wod__ctx.arenaroot = ArenaRoot_create(1024*1024);
        if (wod__ctx.arenaroot.buf == NULL) {
            printferr("Couldn't create arena. No memory?");
            wassert(false);
        }
    }
    return ArenaRoot_get_arena(wod__ctx.arenaroot);
}


int wod_get_error(void) {
    int err = wod__ctx.error;
    wod__ctx.error = 0;
    return err;
}

void wod__set_error(int err) {
    wod__ctx.error = err;
}

Image load_image(strview_t path) {
    Arena scratch = wod__get_arena();
    strbuf_t *path_buf = strbuf_create_with_arena(path, &scratch);
    wod__set_error(0);
    Image img = { 0 };
    img.pixels = (uint32_t *)stbi_load(path_buf->cstr, &img.size.x, &img.size.y, NULL, STB_RGBA);
    if (img.pixels == NULL) {
        printfd("ERROR: Could not load file `"PRIstrw"`: %s\n", PRIstrarg(path), stbi_failure_reason());
        wod__set_error(-1);
    }
    return img;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//typedef void (*wod_draw_rect_t)    (Rect2i rect, Color color);

typedef void (*wod_set_buffer_t)   (u32 *pixels, V2i size, int stride);
typedef void (*wod_draw_rect_t)    (Rect2i rect, Color color);
typedef void (*wod_draw_texture_t) (Image img, Rect2i source, Rect2i dest);
//typedef void (*wod_DrawRectLinesCallback_t) (Rect2i rect, Color color, int thickness);
//typedef void (*wod_DrawTextCallback_t) (strview_t str, intptr_t font, V2i position, int font_size, int spacing, int textLineSpacing, Color tint);
//typedef void (*wod_ScissorCallback_t) (bool start_end, Rect2i rect);

typedef struct {
    wod_set_buffer_t   set_buffer;
    wod_draw_rect_t    draw_rect;
    wod_draw_texture_t draw_texture;
} Drawer;

Drawer wod__drawer;

void wod_set_drawer(Drawer drawer) {
    wassert(drawer.set_buffer);
    wassert(drawer.draw_rect);
    wassert(drawer.draw_texture);
    wod__drawer = drawer;
}


void wod_set_buffer(u32 *pixels, V2i size, int stride) {
    wod__drawer.set_buffer(pixels, size, stride);
}

void draw_rect(Rect2i rect, Color color) {
    wod__drawer.draw_rect(rect, color);
}

void draw_image(Image img, Rect2i source, Rect2i dest) {
    wod__drawer.draw_texture(img, source, dest);
}

/*
   note: make_draw_buffer_drawer() {
    return (draw_buffer_drawer_t) {
        .rect_callback = draw_rect
    }
   }

   */



#endif
