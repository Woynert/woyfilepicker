#ifndef RAYLIB_EXTRA2_H
#define RAYLIB_EXTRA2_H

#include "raylib_drawbuffer.h"

void b_DrawCheckerboard(Rect2i rect, Color color, int square_length) {
    int h_count = (int)ceilf(((float)rect.width / (float)square_length) / 2.f);
    int v_count = (int)ceilf(((float)rect.height / (float)square_length));
    int h_offset = 0, h_left, v_left = rect.height;
    for (int j = 0; j < v_count; ++j) {
        h_offset = !h_offset;
        h_left = rect.width - h_offset *square_length;
        for (int i = 0; i < h_count; ++i) {
            b_DrawRectangle((Rect2i){{
                rect.x + i * square_length *2 + h_offset * square_length,
                rect.y + j * square_length,
                int_min(square_length, h_left),
                int_min(square_length, v_left),
                }}, color
            );
            h_left -= square_length *2;
        }
        v_left -= square_length;
    }
}

void b_DrawRectangleLinesEx(Rect2i rec, int lineThick, Color color) {
    // Blatantly stolen from rshapes.c.
    if ((lineThick > rec.width) || (lineThick > rec.height))
    {
        if (rec.width >= rec.height) lineThick = rec.height/2;
        else if (rec.width <= rec.height) lineThick = rec.width/2;
    }
    Rect2i top = {{ rec.x, rec.y, rec.width, lineThick }};
    Rect2i bottom = {{ rec.x, rec.y - lineThick + rec.height, rec.width, lineThick }};
    Rect2i left = {{ rec.x, rec.y + lineThick, lineThick, rec.height - lineThick*2 }};
    Rect2i right = {{ rec.x - lineThick + rec.width, rec.y + lineThick, lineThick, rec.height - lineThick*2 }};
    b_DrawRectangle(top, color);
    b_DrawRectangle(bottom, color);
    b_DrawRectangle(left, color);
    b_DrawRectangle(right, color);
    //DrawRectangleLinesEx(
}

/*
void b_DrawRectangleLinesEx_f(Rect2 rec, float lineThick, Color color) {
    if ((lineThick > rec.width) || (lineThick > rec.height))
    {
        if (rec.width >= rec.height) lineThick = rec.height/2;
        else if (rec.width <= rec.height) lineThick = rec.width/2;
    }
    Rect2 top = {{ rec.x, rec.y, rec.width, lineThick }};
    Rect2 bottom = {{ rec.x, rec.y - lineThick + rec.height, rec.width, lineThick }};
    Rect2 left = {{ rec.x, rec.y + lineThick, lineThick, rec.height - lineThick*2 }};
    Rect2 right = {{ rec.x - lineThick + rec.width, rec.y + lineThick, lineThick, rec.height - lineThick*2 }};
    b_DrawRectangleRec(top, color);
    b_DrawRectangleRec(bottom, color);
    b_DrawRectangleRec(left, color);
    b_DrawRectangleRec(right, color);
}
*/

#endif
