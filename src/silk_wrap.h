#ifndef SILK_WRAP_H
#define SILK_WRAP_H

#include "la_extra.h"
#include "wod_drawer.h"
#include "silk.h"


typedef struct SilkCtx {
    union {
        char *buffer;
        pixel *pixels;
    };
    int stride;
    V2i viewport_size;
} SilkCtx;

SilkCtx silkwrap_ctx = { 0 };


void silkwrap_set_buffer(u32 *pixels, V2i size, int stride) {
    silkwrap_ctx.pixels = pixels;
    silkwrap_ctx.viewport_size = size;
    silkwrap_ctx.stride = stride;
}

void silkwrap_draw_rect(Rect2i rect, Color color) {
    silkDrawRect(
        (pixel*)silkwrap_ctx.buffer,
        (vec2i){silkwrap_ctx.viewport_size.x, silkwrap_ctx.viewport_size.y},
        silkwrap_ctx.stride,
        (vec2i) { rect.x, rect.y },
        (vec2i) { rect.width, rect.height },
        color.rgba
    );
}

//SILK_API i32 silkDrawImageScaled(image* img, vec2i position, vec2i size_dest);
//SILK_API i32    silkDrawImagePro(image* img, vec2i position, vec2i offset, vec2i size_dest, pixel tint);
void silkwrap_draw_texture(Image img, Rect2i source, Rect2i dest) {
    image silk_image = silkBufferToImage(img.pixels, (vec2i){img.size.x, img.size.y});
    silkDrawImagePro(
        (pixel*)silkwrap_ctx.buffer,
        (vec2i){silkwrap_ctx.viewport_size.x, silkwrap_ctx.viewport_size.y},
        silkwrap_ctx.stride,
        &silk_image,
        (vec2i){dest.x, dest.y},
        (vec2i){0, 0}, // offset
        //(vec2i){source.x, source.y}, // offset
        (vec2i){dest.width, dest.height},
        WHITE.rgba
    );
}

Drawer silkwrap_make_drawer(void) {
    return (Drawer) {
        .draw_rect = silkwrap_draw_rect,
        .draw_texture = silkwrap_draw_texture,
    };
}

#endif
