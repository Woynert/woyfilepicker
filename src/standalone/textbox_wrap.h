#ifndef TEXTBOX_WRAP
#define TEXTBOX_WRAP

// ↓↓↓ PLATFORM SPECIFIC CODE GOES HERE ↓↓↓

#include <GLFW/glfw3.h>
#include "la_extra.h"
#include "drawbuffer.h"
#include "textbox.h"
#include "ui_common.h"

void textbox_glfw_key_callback(Textbox *t, int key, int scancode, int action, int mods) {
    (void)scancode, (void)mods;
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_DELETE:
        {
            textbox__delete(t);
            textbox__debug_print(t);
            break;
        }
        case GLFW_KEY_BACKSPACE:
        {
            textbox__backspace(t);
            textbox__debug_print(t);
            break;
        }
        case GLFW_KEY_LEFT:
        {
            textbox__cursor_left(t);
            textbox__debug_print(t);
            break;
        }
        case GLFW_KEY_RIGHT:
        {
            textbox__cursor_right(t);
            textbox__debug_print(t);
            break;
        }
        default: { } }
    }
}


void textbox_click(Textbox *t, wod_font_t font, V2i text_pos, int spacing, V2i mouse_pos) {
    int cursor = text_get_nearest_codepoint_to_px(mouse_pos, text_pos, font, (strview_t){.data=t->buffer,.size=t->size}, spacing);
    printfd("Got cursor from click %d <---", cursor);
    textbox_set_cursor(t, cursor);
}


void textbox_draw(Textbox *t, Rect2i rect, wod_font_t font, int font_size, int *scroll_px, float *vel_px) {
    enum { PAD = 2, TEXT_END_PAD = 10 };
    Rect2i area = Rect2i_add_padding_all(rect, PAD);
    strview_t text_until_cursor = { .data=t->buffer, .size=t->cursor }; 
    strview_t text = { .data=t->buffer, .size=t->size }; 
    int char_space = 0, line_space = 0;
    #define DRAW_TEXT_ARGS font_size, char_space, line_space
    Rect2i text_size_cursor = text_measure(font, text_until_cursor, area.pos, DRAW_TEXT_ARGS);
    Rect2i text_size = text_measure(font, text, area.pos, DRAW_TEXT_ARGS);
    Rect2i cursor_rect = {{ area.x + text_size_cursor.width, area.y, 2, font_size }};

    ui__calculate_fancy_scroll_px_with_focus(scroll_px, vel_px,
            area.width, text_size.width + TEXT_END_PAD, 0,
            true, text_size_cursor.width, cursor_rect.width);

    b_draw_frame(rect, BLACK, 1);
    b_draw_begin_scissor(area);
    b_draw_text_ext(font, text, v2i(area.pos.x + *scroll_px, area.pos.y), DRAW_TEXT_ARGS, BLACK);
    cursor_rect.x += *scroll_px;
    b_draw_rect(cursor_rect, BLUE);
    b_draw_end_scissor();
    #undef DRAW_TEXT_ARGS
}

void textbox_free(Textbox *t) {
    if (t->buffer) { free(t->buffer); }
    *t = (Textbox) { 0 };
}

// ↑↑↑ PLATFORM SPECIFIC CODE  ↑↑↑

#endif // TEXTBOX_WRAP
