#ifndef WOD_H
#define WOD_H

#include "stb_image.h"
#include "stb_truetype.h"
#include "arena.h"
#include "arena_extra.h"
#include "strview.h"
#include "wstrview.h"
#include "strbuf_extra.h"
#include "strbuf.h"
#include "portable_utils.h"
#include "la_extra.h"
#include "raylib_lite.h"
#include "stbtt_extra.h"
#include <stdint.h>

#define STB_RGBA 4

typedef uint8_t  u8;
typedef int32_t  i32;
typedef uint32_t u32;
typedef float    f32;

typedef union {
    strview_t view;
    struct {
        const char* data;
        int size;
    };
} wod_file_t;

typedef struct {
    V2i size;
    union {
        char *data;
        u32 *pixels; // Always rgba.
    };
} Image;

typedef struct {
    int font_size;

    //const char *bitmap;
    Image bitmap;
    stbtt_packedchar* packed_char_info;
    int packed_char_info_size;

    // Codepoint range is inclusive.
    V2i *ranges;
    int ranges_size;
} wod_font_t;

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


int wod_get_error(void) { // TODO: DEPRECATE ME
    int err = wod__ctx.error;
    wod__ctx.error = 0;
    return err;
}
int wod_error(void) {
    int err = wod__ctx.error;
    wod__ctx.error = 0;
    return err;
}

void wod__set_error(int err) {
    wod__ctx.error = err;
}

// Prefer to call load_file_str.
wod_file_t load_file(const char *path) {
    char *buffer = NULL;
    FILE *file = fopen(path, "rb");
    if (!file) { goto exit_abort; }
    int err = fseek(file, 0, SEEK_END);
    if (err != 0) { goto exit_abort; }
    int size = 0;
    {
        long long_size = ftell(file);
        if (long_size > INT_MAX) {
            printfd("ERR: File ("PRIbyte") is larget than INT_MAX ("PRIbyte")", PRIbytearg(long_size), PRIbytearg(INT_MAX));
            goto exit_abort;
        }
        size = (int)long_size;
    }
    fseek(file, 0, SEEK_SET);
    buffer = (char*)malloc((size_t)size);
    if (!buffer) { goto exit_abort; }
    unsigned long bytes_read = fread(buffer, 1, (size_t)size, file); // Read 1 byte size times.
    if (bytes_read != (unsigned long)size) { goto exit_abort; }
    if ((0)) {
        exit_abort:
        if (buffer) { free(buffer); }
        wod__set_error(-1);
        return (wod_file_t) { 0 };
    }
    if (file) { fclose(file); }
    wod__set_error(0);
    return (wod_file_t) { .data = buffer, .size = size, };
}

wod_file_t load_file_str(const strview_t path, Arena scratch) {
    strbuf_t *path_buf = strbuf_create_with_arena(path, &scratch);
    return load_file(path_buf->cstr);
}

void free_file(wod_file_t file) {
    if (file.data) {
        free((void*)file.data);
    }
    file = (wod_file_t) { 0 };
}

Image load_image(strview_t path) {
    wod__set_error(0);
    Arena scratch = wod__get_arena();
    strbuf_t *path_buf = strbuf_create_with_arena(path, &scratch);
    Image img = { 0 };
    img.pixels = (uint32_t *)stbi_load(path_buf->cstr, &img.size.x, &img.size.y, NULL, STB_RGBA);
    if (img.pixels == NULL) {
        printfd("ERROR: Could not load file `"PRIstrw"`: %s\n", PRIstrarg(path), stbi_failure_reason());
        wod__set_error(-1);
    }
    return img;
}

void free_image(Image img) {
    if (img.data) {
        stbi_image_free(img.data);
    }
    img = (Image) { 0 };
}

wod_font_t load_font(Arena scratch, strview_t font_path, int font_size, V2i *ranges, int range_count) {
    char *bitmap = NULL;
    wod_font_t font = { 0 };
    wod_file_t file = load_file_str(font_path, scratch);
    if (wod_error()) { goto quit_abort; }

    int font_index = 0;
    int padding = 1;
    int width = 300;
    int height = 0;

    stbtt_pack_range *pack_ranges = make_stbtt_pack_range(font_size, ranges, range_count, &scratch);

    int err = stbtt_packed_bitmap_calculate_minimum_height_for_given_width(
            scratch, width, &height, padding, (unsigned char*)file.data, font_index, pack_ranges, range_count);
    if (err) { goto quit_abort; }

    bitmap = malloc_new(char, width * height);
    err = stbtt_create_bitmap_ranges((unsigned char*)file.data, font_index, padding, (unsigned char*)bitmap, width, height, 0, pack_ranges, range_count);
    if (err) { goto quit_abort; }

    // Success: Now copy data to final storage.
    {
        font.packed_char_info_size = 0;
        for (int i = 0; i < range_count; ++i) {
            font.packed_char_info_size += ranges[i].c[1] - ranges[i].c[0] +1; // Inclusive
        }

        font.packed_char_info = malloc_new(stbtt_packedchar, font.packed_char_info_size);
        if (!font.packed_char_info) { goto quit_abort; }

        int offset = 0;
        for (int i = 0; i < range_count; ++i) {
            stbtt_pack_range *range = &pack_ranges[i];
            for (int k = 0; k < range->num_chars; ++k) {
                font.packed_char_info[offset + k] = range->chardata_for_range[k];
            }
            offset += range->num_chars;
        }

        //font.ranges = (int (*)[2])malloc(sizeof(V2i) * (size_t)(range_count)); // This looks ugly.
        font.ranges = malloc_new(V2i, range_count);
        if (!font.ranges) { goto quit_abort; }
        memcpy(font.ranges, ranges, (size_t)range_count * sizeof(V2i));
    }

    if ((0)) {
        quit_abort:
        if (bitmap)                { free((void*)bitmap); }
        if (font.ranges)           { free((void*)font.ranges); }
        if (font.packed_char_info) { free((void*)font.packed_char_info); }
        free_file(file);
        wod__set_error(-1);
        return (wod_font_t) { 0 };
    }

    free_file(file);
    wod__set_error(0);
    return (wod_font_t) {
        .font_size             = font_size,
        .bitmap                = (Image) { .size=(V2i){{ width, height}}, .data=bitmap, },
        .packed_char_info      = font.packed_char_info,
        .packed_char_info_size = font.packed_char_info_size,
        .ranges                = font.ranges,
        .ranges_size           = range_count,
    };
}

void free_font(wod_font_t font) {
    free_image(font.bitmap);
    if (font.ranges)           { free((void*)font.ranges); }
    if (font.packed_char_info) { free((void*)font.packed_char_info); }
    font = (wod_font_t) { 0 };
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
typedef void (*wod_draw_texture_bitmap_t) (Image img, Rect2i source, Rect2i dest, Color tint);
//typedef void (*wod_DrawRectLinesCallback_t) (Rect2i rect, Color color, int thickness);
//typedef void (*wod_DrawTextCallback_t) (strview_t str, intptr_t font, V2i position, int font_size, int spacing, int textLineSpacing, Color tint);
//typedef void (*wod_ScissorCallback_t) (bool start_end, Rect2i rect);

typedef struct {
    wod_set_buffer_t   set_buffer;
    wod_draw_rect_t    draw_rect;
    wod_draw_texture_t draw_texture;
    wod_draw_texture_bitmap_t draw_texture_bitmap;
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

void draw_image_ext(Image img, Rect2i source, Rect2i dest) {
    wod__drawer.draw_texture(img, source, dest);
}

void draw_image(Image img, V2i pos) {
    wod__drawer.draw_texture(img, (Rect2i){.size=img.size}, (Rect2i){.pos=pos,.size=img.size});
}

/// @Returns NULL if not found.
stbtt_packedchar* font_get_codepoint_info(wod_font_t font, int codepoint) {
    int index_offset = 0;
    for (int i = 0; i < font.ranges_size; ++i) {
        V2i range = font.ranges[i];
        int start = range.c[0];
        int end = range.c[1];
        //printfd("RANGE start %d end %d", start, end);
        if (int_in_range_inclusive(start, end, codepoint)) {
            int idx = index_offset + (codepoint - start);
            if (int_in_range_inclusive(0, font.packed_char_info_size-1, idx)) {
                return &font.packed_char_info[idx];
            } else {
                printferr("Codepoint(%d) int range(%d,%d) but not found", codepoint, start, end);
                return NULL;
            }
        }
        index_offset += end - start +1;
    }
    return NULL;
}

void draw_text(wod_font_t font, const strview_t text, V2i pos) {
    int xoffset = 0;
    int baseline = pos.y + font.font_size;

    char* bytes = (char*)text.data;
    int available_bytes = text.size;
    int codepoint_size = 0;
    int codepoint;

    bool try_get_callback = true;
    stbtt_packedchar *cp_info_fallback = NULL;

    while (available_bytes > 0) {
        codepoint = GetCodepointNext_woy(bytes, &codepoint_size, available_bytes);
        available_bytes -= codepoint_size;
        bytes += codepoint_size;
        printfd("Codepoint %lc", codepoint);

        stbtt_packedchar *cp_info = font_get_codepoint_info(font, codepoint);
        if (!cp_info) { // ↓↓↓ This feels to noisy, consider just drawing a rectangle instead.
            if (cp_info_fallback) {
                cp_info = cp_info_fallback;
            } else if (try_get_callback) {
                try_get_callback ^= 1;
                cp_info_fallback = font_get_codepoint_info(font, 0xFFFD); //'�'
                if (!try_get_callback) {
                    cp_info_fallback = font_get_codepoint_info(font, (int)'?');
                }
                cp_info = cp_info_fallback;
            }
            if (!cp_info) { continue; }
        }

        //printfd("%d %d %d %d", cp_info->x0, cp_info->x1, cp_info->y0, cp_info->y1);
        printfd("%f %f %f %f %f",
                cp_info->xoff,
                cp_info->yoff,
                cp_info->xadvance,
                cp_info->xoff2,
                cp_info->yoff2
        );
        Rect2i rect = (Rect2i) {{ cp_info->x0, cp_info->y0, cp_info->x1 - cp_info->x0, cp_info->y1 - cp_info->y0 }};
        wod__drawer.draw_texture_bitmap(font.bitmap,
                rect,
                (Rect2i) {{ pos.x +xoffset +(int)roundf(cp_info->xoff), baseline + (int)roundf(cp_info->yoff), rect.width, rect.height }},
                RED
            );
        //xoffset += cp_info->x1 - cp_info->x0;
        //roun
        xoffset += (int)roundf(cp_info->xadvance);
    }
}

/*
   note: make_draw_buffer_drawer() {
    return (draw_buffer_drawer_t) {
        .rect_callback = draw_rect
    }
   }

   */



#endif
