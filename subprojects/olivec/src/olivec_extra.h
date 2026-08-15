#ifndef OLIVEC_EXTRA_H
#define OLIVEC_EXTRA_H

#define OLIVEC_BITMAP_NULL ((Olivec_Bitmap) {0})

typedef struct {
    char *pixels;
    size_t width;
    size_t height;
    size_t stride;
} Olivec_Bitmap;

OLIVECDEF Olivec_Bitmap olivec_bitmap(char *pixels, size_t width, size_t height, size_t stride);
OLIVECDEF Olivec_Bitmap olivec_subbitmap(Olivec_Bitmap oc, int x, int y, int w, int h);
OLIVECDEF void olivec_bitmap_blend(Olivec_Canvas oc, int x, int y, int w, int h, Olivec_Bitmap bitmap, uint32_t tint);

#endif // OLIVEC_EXTRA_H
#ifdef OLIVEC_IMPLEMENTATION

OLIVECDEF Olivec_Bitmap olivec_bitmap(char *pixels, size_t width, size_t height, size_t stride)
{
    Olivec_Bitmap oc = {
        .pixels = pixels,
        .width  = width,
        .height = height,
        .stride = stride,
    };
    return oc;
}

OLIVECDEF Olivec_Bitmap olivec_subbitmap(Olivec_Bitmap oc, int x, int y, int w, int h)
{
    Olivec_Normalized_Rect nr = {0};
    if (!olivec_normalize_rect(x, y, w, h, oc.width, oc.height, &nr)) return OLIVEC_BITMAP_NULL;
    oc.pixels = &OLIVEC_PIXEL(oc, nr.x1, nr.y1);
    oc.width = nr.x2 - nr.x1 + 1;
    oc.height = nr.y2 - nr.y1 + 1;
    return oc;
}

OLIVECDEF void olivec_bitmap_blend(Olivec_Canvas oc, int x, int y, int w, int h, Olivec_Bitmap sprite, uint32_t tint)
{
    if (sprite.width == 0) return;
    if (sprite.height == 0) return;

    Olivec_Normalized_Rect nr = {0};
    if (!olivec_normalize_rect(x, y, w, h, oc.width, oc.height, &nr)) return;

    int xa = nr.ox1;
    if (w < 0) xa = nr.ox2;
    int ya = nr.oy1;
    if (h < 0) ya = nr.oy2;
    for (int y = nr.y1; y <= nr.y2; ++y) {
        for (int x = nr.x1; x <= nr.x2; ++x) {
            size_t nx = (x - xa)*((int) sprite.width)/w;
            size_t ny = (y - ya)*((int) sprite.height)/h;
            char value = OLIVEC_PIXEL(sprite, nx, ny);
            olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), OLIVEC_RGBA(OLIVEC_RED(tint), OLIVEC_RED(tint), OLIVEC_RED(tint), value));
        }
    }
}

#endif // OLIVEC_IMPLEMENTATION
