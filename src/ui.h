#ifndef UI_H
#define UI_H

#include "drawbuffer.h"
#include "la_extra.h"
#include "la.h"
#include "portable_utils.h"
#include "state_init.h"
#include "uitree.h"
#include "state.h"
#include "ui_common.h"
#include "wod_drawer.h"
#include "x11back.h"


#define DEFAULT_BG GRAY
#define DEFAULT_FG BLACK
#define CON_PAD 2


#define WIDGET__TABLE \
X( UI_WIDGET_TEST               , ui_widget_test              ) \
X( UI_WIDGET_HEADER             , ui_widget_header              ) \
X( UI_WIDGET_BOOKMARKS          , ui_widget_bookmarks          ) \
X( UI_WIDGET_EXPLORER           , ui_widget_explorer          ) \
X( UI_WIDGET_HSPLIT_DRAG        , ui_widget_hsplit_drag          ) \
X( UI_WIDGET_VSPLIT_DRAG        , ui_widget_vsplit_drag          ) \
/*
X( UI_WIDGET_VSPLIT_DRAG          , ui_widget_vsplit_drag          ) \
X( UI_WIDGET_3HSPLIT_DRAG         , ui_widget_3hsplit_drag         ) \
*/

enum UI_WIDGET {
    #define X(A, ...) A,
    WIDGET__TABLE
    #undef X
    UI_WIDGET_AMOUNT
};

char* UI_WIDGET_STR[] = {
    #define X(A, ...) #A,
    WIDGET__TABLE
    #undef X
};

#define X(A, B) void B (Ctx *ctx, uitree_DrawInfo info);
WIDGET__TABLE
#undef X

void (*widget_func[]) (Ctx *ctx, uitree_DrawInfo info) = {
    #define X(A, B) B,
    WIDGET__TABLE
    #undef X
};

int ui_line_height(Ctx *ctx) {
    return ctx->font1.font_size;
}

void ui_draw_text(Ctx *ctx, strview_t text, V2i pos) {
    b_draw_text(ctx->font1, text, pos, ctx->font1.font_size, DEFAULT_FG);
}

bool ui_button(Rect2i rect) {
    if (Rect2i_collides_V2i(rect, winput_mouse_pos())) {
        b_draw_rect(rect, BLUE);
        if (mice_pressed(MouseLeft)) {
            mice_pressed_consume(MouseLeft);
            return true;
        }
    }
    return false;
}

void ui_widget_test (Ctx *ctx, uitree_DrawInfo info) {
    //info.area.height /= 2;
    //info.area.height /= 10;
    //b_draw_rect(info.area, GREEN);

    //Rect2i btn = {{ 20, 20, 100, 50 }};
    //V2i mouse = winput_mouse_pos();

    //if (Rect2i_collides_V2i(btn, mouse)) {
        //b_draw_rect(btn, BLUE);
    //} else {
        //b_draw_rect(btn, GRAY);
    //}
    //b_draw_frame(btn, MAGENTA, 1);

    //b_draw_frame((Rect2i){{mouse.x, mouse.y, 50, 50}}, BLUE, 1);
}

void ui_widget_header (Ctx *ctx, uitree_DrawInfo info) {
    int icon_length = ui_line_height(ctx) * 2;
    Rect2i icon_rect = {{ info.area.x, info.area.y, icon_length, icon_length }};

    b_draw_frame(icon_rect, MAGENTA, 1);
    ui_draw_text(ctx, cstr_SL("^"), icon_rect.pos);

    icon_rect.x += icon_rect.width + CON_PAD;
    b_draw_frame(icon_rect, MAGENTA, 1);
    ui_draw_text(ctx, cstr_SL("<"), icon_rect.pos);

    icon_rect.x += icon_rect.width + CON_PAD;
    b_draw_frame(icon_rect, MAGENTA, 1);
    ui_draw_text(ctx, cstr_SL(">"), icon_rect.pos);

    icon_rect.x += icon_rect.width + CON_PAD;
    icon_rect.width = info.area.width - icon_rect.x - icon_rect.width - CON_PAD;
    b_draw_frame(icon_rect, MAGENTA, 1);

    icon_rect.x += icon_rect.width + CON_PAD;
    icon_rect.width = icon_rect.height;
    b_draw_frame(icon_rect, MAGENTA, 1);
    ui_draw_text(ctx, cstr_SL("search"), icon_rect.pos);
}

void ui_widget_bookmarks (Ctx *ctx, uitree_DrawInfo info) {
    b_draw_frame(info.area, BLUE, 1);
    b_draw_begin_scissor(info.area);
    int line_height = ui_line_height(ctx);
    int *scroll_px = &info.state->int_a;
    float *vel_px = &info.state->float_a;
    Rect2i file_rect = {{ info.area.x, info.area.y, info.area.width, line_height }};
    ui__calculate_fancy_scroll_px(scroll_px, vel_px, info.area.height, ctx->bookmarks.size * file_rect.height, mice_wheel());
    if (vel_px != 0) { MUST_REDRAW = true; }
    file_rect.y += *scroll_px;
    for (dyna_foreach(File, iter, ctx->bookmarks)) {
        File *file = iter.ref;
        if (ui_button((Rect2i){.pos=v2i(file_rect.pos.x,file_rect.pos.y+1),.size=v2i(file_rect.width,file_rect.height-1)})) {
            printfd("Navigating to ...");
        }
        Rect2i icon_rect = {{ file_rect.x + CON_PAD, file_rect.y, file_rect.height, file_rect.height }};
        b_draw_texture(ctx->icon_folder, icon_rect);
        V2i text_pos = {{ icon_rect.x + icon_rect.width + CON_PAD, file_rect.y }};
        ui_draw_text(ctx, File_get_bookmark_alias(*file), text_pos);
        file_rect.pos.y += line_height;
    }
    b_draw_end_scissor();
}

void ui_widget_explorer (Ctx *ctx, uitree_DrawInfo info) {
    b_draw_frame(info.area, BLUE, 1);
}

void draw_all(Ctx *ctx, const bool force_redraw) {

    static int redraw_count = 0;
    static Uitree tree = { 0 };
    static Uitree *t = &tree;
    static bool setup = false;
    if (!setup) { setup = true; uitree_create(t); }

    {
        uitree_build_start(t, (Rect2i){ .size=ctx->window_size });
        uitree_Node widget;
        uitree_Node con_tree = uitree_container_dumb(widget_stack);
        {
            uitree_Node con_vsplit = uitree_container_dumb(widget_vsplit);
            widget_2split_set_user_default_state(t, &con_vsplit, ui_line_height(ctx) * 2, true, 2, 1);
            {
                widget = uitree_widget(UI_WIDGET_HEADER);
                uitree_container_add_child(t, &con_vsplit, widget);
            }
            {
                uitree_Node con_hsplit = uitree_container(t, cstr_SL("hsplit"), widget_hsplit, UI_WIDGET_HSPLIT_DRAG);
                widget_2split_set_user_default_state(t, &con_hsplit, ui_line_height(ctx) * 2, true, CON_PAD*2, 1);
                {
                    {
                        widget = uitree_widget_id(t, UI_WIDGET_BOOKMARKS, cstr_SL("bookmarks"));
                        uitree_container_add_child(t, &con_hsplit, widget);
                    }
                    {
                        widget = uitree_widget_id(t, UI_WIDGET_EXPLORER, cstr_SL("explorer"));
                        uitree_container_add_child(t, &con_hsplit, widget);
                    }
                }
                uitree_container_add_child(t, &con_vsplit, con_hsplit);
            }
            uitree_container_add_child(t, &con_tree, con_vsplit);
        }
        t->root_node = con_tree;
        uitree_build_end(t);
    }

    dbuf_draw_start();
    b_draw_rect((Rect2i) {.size=ctx->window_size}, DARKPURPLE); // Background.
    //b_draw_rect((Rect2i) {.size=ctx->window_size}, DARKPURPLE); // Background.

    int i = -1;
    uitree_List_DrawInfo_It it = { 0 };
    while(uitree_List_DrawInfo_it_next(&t->out_draw_list, &it)) {
        ++i;
        int depth = t->out_draw_list.size - i;
        uitree_DrawInfo draw = *it.item;
        dbuf_set_layer((uint8_t)depth);
        widget_func[draw.user_draw_func_id](ctx, draw);
    }

    //b_draw_text(ctx->font1, cstr_SL("The quick brown fox jumps over the lazy dog éjpyóç"), v2i(0,300), ctx->font1.font_size, MAGENTA);
    //b_draw_text(ctx->font1, SF(&ctx->framearena, "Hello %d", 10), v2i(0,200), ctx->font1.font_size, MAGENTA);
    //b_draw_text(ctx->font1, SC(&ctx->framearena, SF(&ctx->framearena, "%d:%f", 10, 3.4f)), v2i(0,100), ctx->font1.font_size, MAGENTA);

    if (force_redraw || dbuf_do_buffers_differ()) {
        ++redraw_count;
        //draw_rect((Rect2i) {.size=ctx->window_size}, BLACK);
        //printfd(ANSI_MAG"Must redraw. "Bool_Fmt"!!!!!!1", Bool_Arg(dbuf_do_buffers_differ()));
        dbuf_draw_end();
        //drawbuf_swap();
        // TODO NONE OF THIS SHOULD BE HERE.
        draw_rect((Rect2i){.pos=v2i(ctx->window_size.x-50,0),.size=v2i(25,24)}, BLACK);
        draw_text(SF(&ctx->framearena, "%d", redraw_count), ctx->font1, v2i(ctx->window_size.x-50,0), 10, 0, 0, GREEN);
        rgba_to_bgra((u32*)x11_get_buffer(), (Rect2i){.size=ctx->window_size}, ctx->window_size.x);
        char *buffer = x11_swap_buffer();
        wod_set_buffer((u32*)buffer, ctx->window_size, ctx->window_size.x);
    }

    //printfd("Arena consumption is "PRIbyte" out of "PRIbyte, PRIbytearg((1 << 20) - (t->arena.end - t->arena.beg)), PRIbytearg(1 << 20));
}

#endif
