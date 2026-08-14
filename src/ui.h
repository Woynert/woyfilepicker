#ifndef UI_H
#define UI_H

//#include "operations.h"
#include "raylib_drawbuffer.h"
//#include "raylib_extra.h"
//#include "raylib_extra2.h"
#include "la_extra.h"
//#include "raylib.h"
#include "la.h"
//#include "rlgl.h"
#include "portable_utils.h"
#include "uitree.h"
#include "state.h"
#include "ui_common.h"


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
    //b_DrawRect(info.area, GRAY);
    info.area.height /= 2;
    //b_DrawRect(info.area, MAGENTA);
    info.area.height /= 10;
    b_DrawRect(info.area, LIME);
    //b_DrawRectLines(info.area, MAGENTA, 1);
}

void draw_all(Ctx *ctx) {

    static Uitree tree = { 0 };
    static Uitree *t = &tree;
    static bool setup = false;
    if (!setup) { setup = true; uitree_create(t); }

    uitree_build_start(t, (Rect2i){ .size=ctx->window_size });

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
        drawbuf_set_layer((uint8_t)depth);
        widget_func[draw.user_draw_func_id](ctx, draw);
    }
    drawbuf_draw_all();

    //printfd("Arena consumption is "PRIbyte" out of "PRIbyte, PRIbytearg((1 << 20) - (t->arena.end - t->arena.beg)), PRIbytearg(1 << 20));
}

#endif
