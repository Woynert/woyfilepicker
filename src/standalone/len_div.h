#ifndef LEN_DIV_H
#define LEN_DIV_H

#include "la_extra.h"

typedef enum {
    LEN_DIV_PX,
    LEN_DIV_PERCENT_OF_AVAILABLE,
    LEN_DIV_PERCENT_OF_TOTAL,
} LEN_DIV__TYPE;

typedef struct {
    LEN_DIV__TYPE kind;
    int value;
    int minimum;
} len_div_t;

inline static len_div_t len_div_px(int px) {
    return (len_div_t) { .kind = LEN_DIV_PX, .value = px };
}

inline static len_div_t len_div_percent_of_available(int percent, int minimum) {
    return (len_div_t) { .kind = LEN_DIV_PERCENT_OF_AVAILABLE, .value = percent, .minimum = minimum };
}

inline static len_div_t len_div_percent_total(int percent, int minimum) {
    return (len_div_t) { .kind = LEN_DIV_PERCENT_OF_TOTAL, .value = percent, .minimum = minimum };
}

/// @Param pad. Padding to add between rects.
inline static void div_calculate(
    const Rect2i area, const bool horizontal, const int pad,
    Rect2i *out_rects, len_div_t *divs, const int count
) {
    if (count <= 0) { return; }
    int taken = (count - 1) * pad;
    for (int i = 0; i < count; ++i) {
        if (divs[i].kind == LEN_DIV_PERCENT_OF_AVAILABLE) { continue; }
        if (divs[i].kind == LEN_DIV_PERCENT_OF_TOTAL) {
            divs[i].value = int_max(divs[i].minimum, (int)(((float)divs[i].value/100.0) * area.width));
        }
        taken += divs[i].value;
    }
    const int available = area.width - taken;
    for (int i = 0; i < count; ++i) {
        if (divs[i].kind == LEN_DIV_PERCENT_OF_AVAILABLE) {
            divs[i].value = int_max(divs[i].minimum, (int)(((float)divs[i].value/100.0) * available));
        }
    }
    int pos;
    if (horizontal) {
        pos = area.x;
        for (int i = 0; i < count; ++i) {
            out_rects[i] = (Rect2i) {{ pos, area.y, divs[i].value, area.height }};
            pos += pad + out_rects[i].width;
        }
    } else {
        wassert(false); // TODO.
    }
}

#endif
