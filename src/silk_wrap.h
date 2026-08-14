#ifndef SILK_WRAP_H
#define SILK_WRAP_H

#include "silk.h"
#include "raylib_drawbuffer.h"

typedef void (*drawbuf_DrawRectCallback_t) (Rect2i rect, Color color);
typedef void (*drawbuf_DrawRectLinesCallback_t) (Rect2i rect, Color color, int thickness);
typedef void (*drawbuf_DrawTextCallback_t) (strview_t str, intptr_t font, V2i position, int font_size, int spacing, int textLineSpacing, Color tint);
typedef void (*drawbuf_DrawTextureCallback_t) (intptr_t texture, Rect2i source, Rect2i dest, V2i origin, float rotation, Color tint);
typedef void (*drawbuf_ScissorCallback_t) (Rect2i rect, bool start_end);

void *silk__allocator(void* ptr, size_t size, int align, void* user_data) {
    void* result = NULL; (void)align; (void)user_data;
    if     (!size){ free(ptr);                  } // Free:           ptr != NULL && size == 0
    else if(!ptr ){ result = malloc(size);      } // New allocation: ptr == NULL && size > 0
    else          { result = realloc(ptr, size);} // Reallocation:   ptr != NULL && size > 0
    return result; // malloc guarantees alignment.
}

typedef struct SilkCtx {
    union {
        char *buffer;
        pixel *pixels;
    };
    //int buffer_len;
    int stride;
    V2i viewport_size;
} SilkCtx;

SilkCtx *silk_ctx_curr;

void silk_ctx_init(SilkCtx *s) {
    *s = (SilkCtx) { 0 };
}

void silk_set_buffer(SilkCtx *s, char *buff, V2i view_size, int stride) {
    s->buffer = buff;
    s->viewport_size = view_size;
    s->stride = stride;
}
//void silk_resize(SilkCtx *s, V2i size) {
    //wassert(size.x >= 0 && size.y >= 0);
    //int target_size = size.x * size.y * (int)sizeof(pixel);
    //if (target_size > s->buffer_len) {
        //s->buffer = (char*)silk__allocator(s->buffer, (size_t)target_size, 0, 0);
        //if (!s->buffer) { wassert(false); }
    //}
    //s->viewport_size = size;
//}

void silk_DrawRectCallback(Rect2i rect, Color color) {
    SilkCtx *s = silk_ctx_curr;
    silkDrawRect(
        (pixel*)s->buffer,
        (vec2i){s->viewport_size.x, s->viewport_size.y},
        s->stride,
        (vec2i) { rect.x, rect.y },
        (vec2i) { rect.width, rect.height },
        color.rgba
    );
}



#endif
