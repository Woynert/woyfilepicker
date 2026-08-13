/*
   Simple draw buffer for 2D raylib commands:
   */

#ifndef RAYLIB_DRAWBUFFER_H
#define RAYLIB_DRAWBUFFER_H

#include <stdlib.h>
#include "arenady.h"
#include "la_extra.h"
#include "wstrview.h"
#include "raylib_lite.h"

// Finds a multiple of a number which makes it less or equal to a cap.
// @returns Number or 0 if not found.
int find_multiple_max_fit(int n, int cap) {
    if (n <= 0 || cap <= 0) { return 0; }
    return (int)floorf((float)cap / (float)n);
}
V2i Rect_fit_in_Rect_and_preserve_aspect_ratio(V2i container, V2i rect) {
    float scale_x = (float)container.x / (float)rect.x;
    float scale_y = (float)container.y / (float)rect.y;
    float scale_factor = fminf(scale_x, scale_y);
    return (V2i) {{
        .x = (int)((float)rect.x * scale_factor),
        .y = (int)((float)rect.y * scale_factor),
    }};
}

typedef enum {
    DRAWCMD_RECT,
    DRAWCMD_RECT_LINES,
    DRAWCMD_TEXT,
    DRAWCMD_TEXTURE,
    DRAWCMD_BEGIN_SCISSOR,
    DRAWCMD_END_SCISSOR,
} DrawCmd;


#define DYNA__TYPE DrawCmd
#define DYNA__NAMESPACE DrawCmd_da
#include "da.h"


typedef struct drawbuf_Layer {
    ArenaDy arena;
    DrawCmd_da commands;
} drawbuf_Layer;


struct {
    drawbuf_Layer layers[256];
} DrawBuf;
uint8_t DrawBuf__currlayer = 0;


void drawbuf_init(void) {
    for (int i = 0; i < countofi(DrawBuf.layers); ++i) {
        drawbuf_Layer *layer = &DrawBuf.layers[i];
        layer->arena = arenady_create();
        layer->commands = DrawCmd_da_create();
    }
}

void drawbuf_deinit(void) {
    for (int i = 0; i < countofi(DrawBuf.layers); ++i) {
        drawbuf_Layer *layer = &DrawBuf.layers[i];
        arenady_free(&layer->arena);
        DrawCmd_da_free(&layer->commands);
    }
}

void drawbuf_set_layer(uint8_t layer) {
    DrawBuf__currlayer = layer;
}

uint8_t drawbuf_get_layer(void) { return DrawBuf__currlayer; }

typedef struct { Rect2i r; Color color; } drawbuf_DrawRect_t;
typedef struct { Rect2i r; Color color; int thickness; } drawbuf_DrawRectLines_t;
typedef struct { intptr_t font; V2i position; int font_size; int spacing; int textLineSpacing; Color tint; int str_size; char str_data[]; } drawbuf_DrawTextEx_t;
typedef struct { intptr_t texture; Rect2i source; Rect2i dest; V2i origin; float rotation; Color tint; } drawbuf_DrawTexture_t;
typedef struct { Rect2i r; } drawbuf_ScissorMode_t;

typedef void (*drawbuf_DrawRectCallback_t) (Rect2i rect, Color color);
typedef void (*drawbuf_DrawRectLinesCallback_t) (Rect2i rect, Color color, int thickness);
typedef void (*drawbuf_DrawTextCallback_t) (strview_t str, intptr_t font, V2i position, int font_size, int spacing, int textLineSpacing, Color tint);
typedef void (*drawbuf_DrawTextureCallback_t) (intptr_t texture, Rect2i source, Rect2i dest, V2i origin, float rotation, Color tint);
typedef void (*drawbuf_ScissorCallback_t) (Rect2i rect, bool start_end);

drawbuf_DrawRectCallback_t      drawbuf__DrawRectCallback = NULL;
drawbuf_DrawRectLinesCallback_t drawbuf__DrawRectLinesCallback = NULL;
drawbuf_DrawTextCallback_t      drawbuf__DrawTextCallback = NULL;
drawbuf_DrawTextureCallback_t   drawbuf__DrawTextureCallback = NULL;
drawbuf_ScissorCallback_t       drawbuf__ScissorCallback = NULL;

void drawbuf_draw_all(void) {
    for (int i = 0; i < countofi(DrawBuf.layers); ++i) {
        drawbuf_Layer *layer = &DrawBuf.layers[i];
        arenady_reset_beginning(&layer->arena);
        for (dyna_foreach(DrawCmd, iter, layer->commands)) {
            DrawCmd cmd = *iter.ref;
            switch (cmd) {
                case DRAWCMD_RECT:
                {
                    drawbuf_DrawRect_t *args = arenady_new(&layer->arena, drawbuf_DrawRect_t, 1);
                    drawbuf__DrawRectCallback(args->r, args->color);
                } break;
                case DRAWCMD_RECT_LINES:
                {
                    drawbuf_DrawRectLines_t *args = arenady_new(&layer->arena, drawbuf_DrawRectLines_t, 1);
                    drawbuf__DrawRectLinesCallback(args->r, args->color, args->thickness);
                } break;
                case DRAWCMD_TEXT:
                {
                    drawbuf_DrawTextEx_t *args = arenady_new(&layer->arena, drawbuf_DrawTextEx_t, 1);
                    arenady_new(&layer->arena, char, args->str_size);
                    strview_t str = { .data = args->str_data, .size = args->str_size };
                    drawbuf__DrawTextCallback(str, args->font, args->position, args->font_size, args->spacing, args->textLineSpacing, args->tint);
                } break;
                case DRAWCMD_TEXTURE:
                {
                    drawbuf_DrawTexture_t *args = arenady_new(&layer->arena, drawbuf_DrawTexture_t, 1);
                    drawbuf__DrawTextureCallback(args->texture, args->source, args->dest, args->origin, args->rotation, args->tint);
                } break;
                case DRAWCMD_BEGIN_SCISSOR:
                {
                    drawbuf_ScissorMode_t *args = arenady_new(&layer->arena, drawbuf_ScissorMode_t, 1);
                    drawbuf__ScissorCallback(args->r, true);
                } break;
                case DRAWCMD_END_SCISSOR:
                {
                    drawbuf__ScissorCallback((Rect2i){0}, false);
                } break;
            }
        }

        // Reset for next frame.
        arenady_reset_beginning(&layer->arena);
        DrawCmd_da_clear_preserving(&layer->commands);
    }
}

void b_DrawTexturePro(intptr_t texture, Rect2i source, Rect2i dest, V2i origin, float rotation, Color tint) {
    drawbuf_Layer *layer = &DrawBuf.layers[DrawBuf__currlayer];
    drawbuf_DrawTexture_t *args = arenady_new(&layer->arena, drawbuf_DrawTexture_t, 1);
    args->texture = texture;
    args->source = source;
    args->dest = dest;
    args->origin = origin;
    args->rotation = rotation;
    args->tint = tint;
    DrawCmd_da_append(&layer->commands, DRAWCMD_TEXTURE);
}

void b_DrawRect(Rect2i r, Color color) {
    drawbuf_Layer *layer = &DrawBuf.layers[DrawBuf__currlayer];
    drawbuf_DrawRect_t *args = arenady_new(&layer->arena, drawbuf_DrawRect_t, 1);
    args->r = r;
    args->color = color;
    DrawCmd_da_append(&layer->commands, DRAWCMD_RECT);
}

void b_DrawRectLines(Rect2i r, Color color, int thickness) {
    drawbuf_Layer *layer = &DrawBuf.layers[DrawBuf__currlayer];
    drawbuf_DrawRectLines_t *args = arenady_new(&layer->arena, drawbuf_DrawRectLines_t, 1);
    args->r = r;
    args->color = color;
    args->thickness = thickness;
    DrawCmd_da_append(&layer->commands, DRAWCMD_RECT_LINES);
}

void b_DrawTextEx(intptr_t font, const strview_t str, V2i pos, int font_size, int spacing, int textLineSpacing, Color tint) {
    if (str.size == 0 || str.data == NULL) { return; }
    drawbuf_Layer *layer = &DrawBuf.layers[DrawBuf__currlayer];
    // Allocate struct + string in one call.
    drawbuf_DrawTextEx_t *args = (drawbuf_DrawTextEx_t *)
        arenady_alloc(&layer->arena, (i64)(sizeof(drawbuf_DrawTextEx_t) + (size_t)str.size), _Alignof(drawbuf_DrawTextEx_t), 1);
    args->font = font;
    args->position = pos;
    args->font_size = font_size;
    args->spacing = spacing;
    args->textLineSpacing = textLineSpacing;
    args->tint = tint;
    args->str_size = str.size;
    memmove(args->str_data, str.data, (size_t)str.size);
    DrawCmd_da_append(&layer->commands, DRAWCMD_TEXT);
}

void b_BeginScissorMode(Rect2i r) {
    drawbuf_Layer *layer = &DrawBuf.layers[DrawBuf__currlayer];
    drawbuf_ScissorMode_t *args = arenady_new(&layer->arena, drawbuf_ScissorMode_t, 1);
    args->r = r;
    DrawCmd_da_append(&layer->commands, DRAWCMD_BEGIN_SCISSOR);
}

void b_EndScissorMode(void) {
    drawbuf_Layer *layer = &DrawBuf.layers[DrawBuf__currlayer];
    DrawCmd_da_append(&layer->commands, DRAWCMD_END_SCISSOR);
}

#endif
