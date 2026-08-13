#ifndef QUICK_MONITOR
#define QUICK_MONITOR

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include "raylib_extra.h"
#define EMBEDDED_BIN_IMPLEMENTATION__ROBOTO_REGULAR_TTF
#include "robotoregular.ttf.h"

/*
    Usage:

        quickmonitor_draw();
        quickmonitor_line("Hello there %d", 10);
        quickmonitor_line("All my lines here");
*/

static void *quickmonitor__get_string(int count, bool reset)
{
    // Source https://nullprogram.com/blog/2023/09/27/

    enum { BUFFER_CAP = 1024 * 1024 }; // 1 MiB
    static char buffer[BUFFER_CAP];
    static struct { char *beg; char *end; } a = { 0 };
    if (a.beg == NULL || reset) {
        a.beg = buffer;
        a.end = buffer + BUFFER_CAP;
        if (reset) return NULL;
    }

    ptrdiff_t available = a.end - a.beg;
    if (available <= 0 || count > available) {
        printf("OUT OF MEMORY. %s:%d\n", __FILE__, __LINE__);
        abort();
    }
    void *ptr = a.beg;
    a.beg += count;
    return memset(ptr, 0, (size_t)count);
}


static struct {
    int line_height;
    int font_size;
    int char_spacing;
    int line_spacing;
    V2i pos;
    int pad;
    Font font;
} quickmonitor__ctx = { 0 };


static void quickmonitor__ensure_font(void) {
    static bool setup = false;
    if (!setup) {
        setup = true;
        quickmonitor__ctx.font_size = 16;
        quickmonitor__ctx.line_height = quickmonitor__ctx.font_size + 2;
        quickmonitor__ctx.char_spacing = 0;
        quickmonitor__ctx.line_spacing = 1;
        quickmonitor__ctx.pad = 2;
        quickmonitor__ctx.font = LoadFontFromMemory(".ttf",
            (const unsigned char*)ROBOTO_REGULAR_TTF,
            (int)ROBOTO_REGULAR_TTF_len,
            quickmonitor__ctx.font_size, NULL, 0);
    }
}


#define quickmonitor_draw()                                                          \
    do {                                                                             \
        quickmonitor__ensure_font();                                                 \
        quickmonitor__ctx.pos = (V2i){{ quickmonitor__ctx.pad, GetScreenHeight() }}; \
    } while(0)


#define quickmonitor_line(fmt, ...)                                                                \
    do {                                                                                           \
        quickmonitor__ctx.pos.y -= quickmonitor__ctx.line_height;                                  \
        const char *line = TextFormat(fmt, ##__VA_ARGS__);                                         \
                                                                                                   \
        Vector2 measure = MeasureTextEx( quickmonitor__ctx.font, line,                             \
                (float)quickmonitor__ctx.font_size, (float)quickmonitor__ctx.char_spacing);        \
        int line_width_px = (int)measure.x;                                                        \
                                                                                                   \
        DrawRectangleReci((Rect2i){{                                                               \
            .x = quickmonitor__ctx.pos.x - quickmonitor__ctx.pad,                                  \
            .y = quickmonitor__ctx.pos.y,                                                          \
            .width = line_width_px + quickmonitor__ctx.pad *2,                                     \
            .height = quickmonitor__ctx.line_height }}, BLACK );                                   \
                                                                                                   \
        DrawTextEx(quickmonitor__ctx.font, line, v2i_to_Vector2(quickmonitor__ctx.pos),            \
                (float)quickmonitor__ctx.font_size, (float)quickmonitor__ctx.char_spacing, WHITE); \
    } while(0)


#endif // !QUICK_MONITOR
