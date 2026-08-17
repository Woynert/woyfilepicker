#ifndef UI_H
#define UI_H

#include "drawbuffer.h"
#include "la_extra.h"
#include "la.h"
#include "portable_utils.h"
#include "uitree.h"
#include "state.h"
#include "ui_common.h"
#include "wod_drawer.h"
#include "x11back.h"


#define WIDGET__TABLE \
X( UI_WIDGET_TEST                 , ui_widget_test              )
/*
X( UI_WIDGET_SPRITE_LIST          , ui_widget_sprite_list          ) \
X( UI_WIDGET_SPRITE_PREVIEW       , ui_widget_sprite_preview       ) \
X( UI_WIDGET_SPRITESHEET_LIST     , ui_widget_spritesheet_list     ) \
X( UI_WIDGET_SPRITESHEET_VIEWPORT , ui_widget_spritesheet_viewport ) \
X( UI_WIDGET_SPRITESHEET_CURSORS  , ui_widget_spritesheet_cursors  ) \
X( UI_WIDGET_WELCOME_SCREEN       , ui_widget_welcome_screen       ) \
X( UI_WIDGET_SPRITESHEET_HINTS    , ui_widget_spritesheet_hints    ) \
X( UI_WIDGET_VSPLIT_DRAG          , ui_widget_vsplit_drag          ) \
X( UI_WIDGET_3HSPLIT_DRAG         , ui_widget_3hsplit_drag         ) \
X( UI_WIDGET_FLOATING_MENU        , ui_widget_floating_menu        ) \
X( UI_WIDGET_STATUS_BAR           , ui_widget_status_bar           )
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

void ui_widget_test (Ctx *ctx, uitree_DrawInfo info) {
    info.area.height /= 2;
    info.area.height /= 10;
    b_draw_rect(info.area, GREEN);

    Rect2i btn = {{ 20, 20, 100, 50 }};
    V2i mouse = winput_mouse_pos();

    if (Rect2i_collides_V2i(btn, mouse)) {
        b_draw_rect(btn, BLUE);
    } else {
        b_draw_rect(btn, GRAY);
    }
    b_draw_frame(btn, MAGENTA, 1);

    b_draw_frame((Rect2i){{mouse.x, mouse.y, 50, 50}}, BLUE, 1);
}

void draw_all(Ctx *ctx, const bool force_redraw) {

    static Uitree tree = { 0 };
    static Uitree *t = &tree;
    static bool setup = false;
    if (!setup) { setup = true; uitree_create(t); }

    uitree_build_start(t, (Rect2i){ .size=ctx->window_size });
    dbuf_draw_start();
    //b_draw_rect((Rect2i) {.size=ctx->window_size}, BLACK); // Background.


    uitree_Node widget;
    uitree_Node con_tree = uitree_container_dumb(widget_stack);
    widget = uitree_widget(UI_WIDGET_TEST);
    uitree_container_add_child(t, &con_tree, widget);
    t->root_node = con_tree;
    uitree_build_end(t);


    int i = -1;
    uitree_List_DrawInfo_It it = { 0 };
    while(uitree_List_DrawInfo_it_next(&t->out_draw_list, &it)) {
        ++i;
        int depth = t->out_draw_list.size - i;
        uitree_DrawInfo draw = *it.item;
        dbuf_set_layer((uint8_t)depth);
        widget_func[draw.user_draw_func_id](ctx, draw);
    }

    b_draw_text(ctx->font1, cstr_SL("The quick brown fox jumps over the lazy dog éjpyóç"), v2i(0,300), ctx->font1.font_size, MAGENTA);

    if (force_redraw || dbuf_do_buffers_differ()) {
        //draw_rect((Rect2i) {.size=ctx->window_size}, BLACK);
        printfd(ANSI_MAG"Must redraw. "Bool_Fmt"!!!!!!1", Bool_Arg(dbuf_do_buffers_differ()));
        dbuf_draw_end();
        //drawbuf_swap();
        // TODO NONE OF THIS SHOULD BE HERE.
        rgba_to_bgra((u32*)x11_get_buffer(), (Rect2i){.size=ctx->window_size}, ctx->window_size.x);
        char *buffer = x11_swap_buffer();
        wod_set_buffer((u32*)buffer, ctx->window_size, ctx->window_size.x);
    }

    //printfd("Arena consumption is "PRIbyte" out of "PRIbyte, PRIbytearg((1 << 20) - (t->arena.end - t->arena.beg)), PRIbytearg(1 << 20));
}

#endif
