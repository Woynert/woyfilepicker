#ifndef OLIVE_C_WRAP_H
#define OLIVE_C_WRAP_H

#include "la_extra.h"
#include "wod_drawer.h"
#define OLIVECDEF
#include "olive.h"

#define OLIVEC_RGBA(r, g, b, a) ((((r)&0xFF)<<(8*0)) | (((g)&0xFF)<<(8*1)) | (((b)&0xFF)<<(8*2)) | (((a)&0xFF)<<(8*3)))

struct {
    Olivec_Canvas canvas;
    bool scissor_enabled;
    Olivec_Canvas canvas_scissor_bk;
} olivewrap_ctx;

void olivewrap_set_buffer(u32 *pixels, V2i size, int stride) {
    olivewrap_ctx.canvas = olivec_canvas(pixels, (size_t)size.x, (size_t)size.y, (size_t)stride);
    olivewrap_ctx.canvas_scissor_bk = olivewrap_ctx.canvas;
    olivewrap_ctx.scissor_enabled = false;
}

void olivewrap_draw_rect(Rect2i rect, Color color) {
    olivec_rect(olivewrap_ctx.canvas, rect.x, rect.y, rect.width, rect.height, color.val);
}

void olivewrap_draw_frame(Rect2i rect, Color color, int tickness) {
    olivec_frame(olivewrap_ctx.canvas, rect.x, rect.y, rect.width, rect.height, (size_t)tickness, color.val);
}

void olivewrap_draw_texture(Image img, Rect2i source, Rect2i dest, V2i origin, float rotation, Color tint) {
    olivec_sprite_blend(olivewrap_ctx.canvas, dest.x, dest.y, dest.width, dest.height,
        olivec_subcanvas(
            olivec_canvas(img.pixels, (size_t)img.size.x, (size_t)img.size.y, (size_t)img.size.x),
            source.x, source.y, source.width, source.height));
    (void)rotation, (void)tint; // TODO: Rotation and tint.
}

void olivewrap_draw_texture_bitmap(Image img, Rect2i source, Rect2i dest, Color tint) {
    olivec_bitmap_blend(olivewrap_ctx.canvas, dest.x, dest.y, dest.width, dest.height,
        olivec_subbitmap(
            olivec_bitmap(img.data, (size_t)img.size.x, (size_t)img.size.y, (size_t)img.size.x),
            source.x, source.y, source.width, source.height),
        tint.val);
}

void olivewrap_scissor(bool start_end, Rect2i rect) {
    olivewrap_ctx.canvas = olivewrap_ctx.canvas_scissor_bk;
    olivewrap_ctx.scissor_enabled = start_end;
    if (start_end) {
        olivewrap_ctx.canvas_scissor_bk = olivewrap_ctx.canvas;
        olivewrap_ctx.canvas = olivec_subcanvas(olivewrap_ctx.canvas, rect.x, rect.y, rect.width, rect.height);
    }
}

Drawer olivewrap_make_drawer(void) {
    return (Drawer) {
        .set_buffer          = olivewrap_set_buffer,
        .draw_rect           = olivewrap_draw_rect,
        .draw_texture        = olivewrap_draw_texture,
        .draw_texture_bitmap = olivewrap_draw_texture_bitmap,
        .draw_frame          = olivewrap_draw_frame,
        .scissor             = olivewrap_scissor,
    };
}

#endif
