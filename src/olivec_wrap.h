#ifndef OLIVE_C_WRAP_H
#define OLIVE_C_WRAP_H

#include "la_extra.h"
#include "wod_drawer.h"
#define OLIVECDEF
#include "olive.h"

Olivec_Canvas olivewrap_canvas = { 0 };

void olivewrap_set_buffer(u32 *pixels, V2i size, int stride) {
    olivewrap_canvas = olivec_canvas(pixels, (size_t)size.x, (size_t)size.y, (size_t)stride);
}

void olivewrap_draw_rect(Rect2i rect, Color color) {
    olivec_rect(olivewrap_canvas, rect.x, rect.y, rect.width, rect.height, color.val);
}

void olivewrap_draw_texture(Image img, Rect2i source, Rect2i dest) {
    olivec_sprite_blend(olivewrap_canvas, dest.x, dest.y, dest.width, dest.height,
        olivec_subcanvas(
            olivec_canvas(img.pixels, (size_t)img.size.x, (size_t)img.size.y, (size_t)img.size.x),
            source.x, source.y, source.width, source.height));
}

void olivewrap_draw_texture_bitmap(Image img, Rect2i source, Rect2i dest, Color tint) {
    olivec_bitmap_blend(olivewrap_canvas, dest.x, dest.y, dest.width, dest.height,
        olivec_subbitmap(
            olivec_bitmap(img.data, (size_t)img.size.x, (size_t)img.size.y, (size_t)img.size.x),
            source.x, source.y, source.width, source.height), tint.val);
}

Drawer olivewrap_make_drawer(void) {
    return (Drawer) {
        .set_buffer = olivewrap_set_buffer,
        .draw_rect = olivewrap_draw_rect,
        .draw_texture = olivewrap_draw_texture,
        .draw_texture_bitmap = olivewrap_draw_texture_bitmap,
    };
}

#endif
