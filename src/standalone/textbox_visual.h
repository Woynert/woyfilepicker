
/*
    * Platform specific code to draw/interact with Textbox through GLFW.
    * Options:
        * TEXTBOX_VISUAL__ONLY_HEADER
        * TEXTBOX_VISUAL__ONLY_IMPLEMENTATION
*/

#include <GLFW/glfw3.h>
#include "la_extra.h"
#include "textbox.h"
#include "wod_drawer.h"

#ifndef TEXTBOX_VISUAL__ONLY_IMPLEMENTATION
#ifndef TEXTBOX_VISUAL__HEADER
#define TEXTBOX_VISUAL__HEADER

typedef struct {
    Rect2i rect;
    int pad;
    int spacing;
    wod_font_t font;
    int scroll_px;
    float vel_px;
    Rect2i measure;
} TextboxVisual;

#endif // #ifndef TEXTBOX_VISUAL__HEADER
#endif // #ifndef TEXTBOX_VISUAL__ONLY_IMPLEMENTATION
#ifndef TEXTBOX_VISUAL__ONLY_HEADER
#ifndef TEXTBOX_VISUAL__IMPLEMENTATION
#define TEXTBOX_VISUAL__IMPLEMENTATION

#include "drawbuffer.h"
#include "ui_common.h"

void TextboxVisual_setup(TextboxVisual *info, wod_font_t font, int pad, int spacing) {
    info->font = font;
    info->pad = pad;
    info->spacing = spacing;
}

void textbox_glfw_key_callback(Textbox *t, int key, int scancode, int action, int mods) {
    (void)scancode;
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_DELETE:
        {
            textbox_delete(t); break;
        }
        case GLFW_KEY_BACKSPACE:
        {
            textbox_backspace(t); break;
        }
        case GLFW_KEY_LEFT:
        {
            if (mods & GLFW_MOD_CONTROL) { textbox_move_by_word(t, -1); }
            else { textbox_cursor_left(t); }
            textbox__debug_print(t);
            break;
        }
        case GLFW_KEY_RIGHT:
        {
            if (mods & GLFW_MOD_CONTROL) { textbox_move_by_word(t, 1); }
            else { textbox_cursor_right(t); }
            textbox__debug_print(t);
            break;
        }
        case GLFW_KEY_PAGE_UP:
        case GLFW_KEY_HOME:
        {
            textbox_set_cursor(t, 0); break;
        }
        case GLFW_KEY_PAGE_DOWN:
        case GLFW_KEY_END:
        {
            textbox_set_cursor(t, INT_MAX); break;
        }
        default: { } }
    }
}


void textbox_click(Textbox *t, const TextboxVisual *info, V2i mouse_pos) {
    V2i draw_pos = v2i(info->rect.x + info->pad + info->scroll_px, info->rect.y);
    int cursor = text_get_nearest_codepoint_to_px(mouse_pos, draw_pos,
            info->font, (strview_t){.data=t->buffer,.size=t->size}, info->spacing);
    textbox_set_cursor(t, cursor);
}


void textbox_draw(Textbox *t, TextboxVisual *info, Rect2i rect) {
    enum { TEXT_END_PAD = 10, SCROLLBAR_HEIGHT = 3 };
    info->rect = rect;
    Rect2i area = Rect2i_add_padding_all(rect, info->pad);
    strview_t text_until_cursor = { .data=t->buffer, .size=t->cursor }; 
    strview_t text = { .data=t->buffer, .size=t->size }; 
    int line_space = 0;
    #define DRAW_TEXT_ARGS info->font.font_size, info->spacing, line_space

    // @Note: I'm measuring the text every frame... Not sure if I like that.
    //

    info->measure = text_measure(info->font, text, area.pos, DRAW_TEXT_ARGS);
    Rect2i text_size_cursor = text_measure(info->font, text_until_cursor, area.pos, DRAW_TEXT_ARGS);
    Rect2i cursor_rect = {{ area.x + text_size_cursor.width, area.y, 2, info->font.font_size }};

    ui__calculate_fancy_scroll_px_with_focus(&info->scroll_px, &info->vel_px,
            area.width, info->measure.width + TEXT_END_PAD, 0,
            true, text_size_cursor.width, cursor_rect.width);

    b_draw_frame(rect, BLACK, 1);
    b_draw_begin_scissor(area);
    b_draw_text_ext(info->font, text, v2i(area.pos.x + info->scroll_px, area.pos.y), DRAW_TEXT_ARGS, BLACK);
    cursor_rect.x += info->scroll_px;
    b_draw_rect(cursor_rect, BLUE);
    #undef DRAW_TEXT_ARGS

    // Draw scrollbar.

    if (info->measure.width > rect.width) {
        Rect2i bar_bg = {{ area.x, area.y + (area.height - SCROLLBAR_HEIGHT), area.width, SCROLLBAR_HEIGHT }};
        Rect2i bar_fg = {{
            bar_bg.x +
            (int)(((float)abs(info->scroll_px) / (float)info->measure.width) * (float)bar_bg.width),
            bar_bg.y,
            (int)(((float)rect.width / (float)info->measure.width) * (float)bar_bg.width),
            SCROLLBAR_HEIGHT
        }};
        b_draw_rect(bar_bg, BLACK);
        b_draw_rect(bar_fg, BLUE);
    }
    b_draw_end_scissor();
}


#endif // #ifndef TEXTBOX_VISUAL__IMPLEMENTATION
#endif // #ifndef TEXTBOX_VISUAL__ONLY_HEADER
#undef TEXTBOX_VISUAL__ONLY_IMPLEMENTATION
#undef TEXTBOX_VISUAL__ONLY_HEADER
