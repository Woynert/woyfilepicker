#ifndef LA_EXTRA
#define LA_EXTRA

#include "la.h"
#include <stdint.h>

typedef uint32_t u32;
typedef uint8_t u8;

typedef union Rect2 {
    struct {
        float x;
        float y;
        float width;
        float height;
    };
    struct {
        V2f pos;
        V2f size;
    };
} Rect2;

typedef union Rect2i {
    struct {
        int x;
        int y;
        int width;
        int height;
    };
    struct {
        V2i pos;
        V2i size;
    };
} Rect2i;

V2f v2f_translate_scale(V2i point, V2i translate, float scale) {
    return v2f_add(
        v2f_mul(
            v2i_2f(point),
            v2ff(scale)
        ),
        v2i_2f(translate)
    );
}

V2i v2i_translate_scale(V2i point, V2i translate, float scale) {
    return v2i_add(
        v2f_2i(v2f_mul(
            v2i_2f(point),
            v2ff(scale)
        )),
        translate
    );
}

Rect2i Rect2i_add_padding(Rect2i r, int top, int right, int bottom, int left) {
    return (Rect2i) {{ r.x + left, r.y + top, r.width - left - right, r.height - top - bottom }};
}

Rect2i Rect2i_add_padding_all(Rect2i r, int pad) {
    return (Rect2i) {{ r.x + pad, r.y + pad, r.width - pad*2, r.height - pad*2 }};
}

bool CheckCollisionPointReci(V2i point, Rect2i rec) {
    return ((point.x >= rec.x) && (point.x < (rec.x + rec.width)) && (point.y >= rec.y) && (point.y < (rec.y + rec.height)));
}

bool Rect2i_collides_V2i (Rect2i rect, V2i point) {
    // (left -X, right +X, up -Y, down +Y)
    int left   = rect.pos.x;
    int right  = rect.pos.x + rect.size.x;
    int top    = rect.pos.y;
    int bottom = rect.pos.y + rect.size.y;
    int x = point.x;
    int y = point.y;
    return ! (
        left > x || x > right ||
        top > y || y > bottom
    );
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



#define Rect2i_Fmt "Rect2i(%d, %d, %d, %d)"
#define Rect2i_Arg(r) r.x, r.y, r.width, r.height

#endif // !LA_EXTRA
