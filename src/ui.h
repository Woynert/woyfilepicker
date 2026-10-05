#ifndef UI_H
#define UI_H

#include "anytype_user_types.h"
#include "drawbuffer.h"
#include "anytype.h"
#include "kinput.h"
#include "textbox_visual.h"
#include "la_extra.h"
#include "la.h"
#include "operations.h"
#include "portable_utils.h"
#include "state_init.h"
#include "uitree.h"
#include "state.h"
#include "ui_common.h"
#include "wod_drawer.h"
#include "x11back.h"
#include "len_div.h"


#define DEFAULT_BG LIGHTGRAY
#define DEFAULT_FG BLACK
#define CON_PAD 2


#define WIDGET__TABLE \
X( UI_WIDGET_NONE               , ui_widget_none              ) \
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
            mice_consume(MouseLeft);
            return true;
        }
    }
    return false;
}

void ui_widget_none (Ctx *ctx, uitree_DrawInfo info) {
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
    const int line_height = ui_line_height(ctx);
    int icon_length = line_height * 2;
    const Rect2i icon_size = {{ 0,0, 32, 32}};
    Rect2i rect = {{ info.area.x, info.area.y, icon_length, icon_length }};

    len_div_t sizes[] = {
        len_div_px(icon_length),
        len_div_px(icon_length),
        len_div_px(icon_length),
        len_div_percent_of_available(50, icon_length),
        len_div_percent_of_available(50, icon_length),
        len_div_px(icon_length),
    };
    Rect2i divs[countofi(sizes)];
    div_calculate(info.area, true, CON_PAD, divs, sizes, countofi(sizes));

    int div_i = 0;
    rect = divs[div_i++];
    if (ui_button(rect)) { navigate_parent_dir(ctx); }
    b_draw_frame(rect, MAGENTA, 1);
    b_draw_texture(ctx->icon_up, Rect2i_center(rect, icon_size));
    {
        strview_t focuser_widget_id = cstr_SL("btn_up");
        if (Rect2i_collides_V2i(rect, winput_mouse_pos())) {
            focuser_take_focus(&ctx->focuser, focuser_widget_id);
            printfd("Cause everytime we touch");
        }
        Focusable btn_focus = focuser_make_focusable(&ctx->focuser, focuser_widget_id);
        if (btn_focus.has_focus) { b_draw_frame(rect, RED, 2); }
    }


    rect = divs[div_i++];
    if (!can_navigate_backwards(ctx)) { b_draw_rect(rect, GRAY); }
    else if (ui_button(rect)) { navigate_backwards(ctx); }
    b_draw_frame(rect, MAGENTA, 1);
    b_draw_texture(ctx->icon_left, Rect2i_center(rect, icon_size));
    {
        strview_t focuser_widget_id = cstr_SL("btn_left");
        if (Rect2i_collides_V2i(rect, winput_mouse_pos())) {
            focuser_take_focus(&ctx->focuser, focuser_widget_id);
        }
        Focusable btn_focus = focuser_make_focusable(&ctx->focuser, focuser_widget_id);
        if (btn_focus.has_focus) { b_draw_frame(rect, RED, 2); }
    }

    rect = divs[div_i++];
    if (!can_navigate_forward(ctx)) { b_draw_rect(rect, GRAY); }
    else if (ui_button(rect)) { navigate_forward(ctx); }
    b_draw_frame(rect, MAGENTA, 1);
    b_draw_texture(ctx->icon_right, Rect2i_center(rect, icon_size));
    {
        strview_t focuser_widget_id = cstr_SL("btn_right");
        if (Rect2i_collides_V2i(rect, winput_mouse_pos())) {
            focuser_take_focus(&ctx->focuser, focuser_widget_id);
        }
        Focusable btn_focus = focuser_make_focusable(&ctx->focuser, focuser_widget_id);
        if (btn_focus.has_focus) { b_draw_frame(rect, RED, 2); }
    }

    rect = divs[div_i++];
    b_draw_frame(rect, MAGENTA, 1);
    textbox_draw(&ctx->tbox_path, &ctx->tbox_path_visual, rect);
    if (mice_pressed(MouseLeft) && Rect2i_collides_V2i(rect, winput_mouse_pos())) {
        textbox_click(&ctx->tbox_path, &ctx->tbox_path_visual, winput_mouse_pos());
    }
    {
        strview_t focuser_widget_id = cstr_SL("tbox_path");
        if (Rect2i_collides_V2i(rect, winput_mouse_pos())) {
            focuser_take_focus(&ctx->focuser, focuser_widget_id);
        }
        Focusable btn_focus = focuser_make_focusable(&ctx->focuser, focuser_widget_id);
        if (btn_focus.has_focus) { b_draw_frame(rect, RED, 2); }
    }

    rect = divs[div_i++];
    b_draw_frame(rect, BLUE, 1);
    textbox_draw(&ctx->tbox_search, &ctx->tbox_search_visual, rect);
    if (mice_pressed(MouseLeft) && Rect2i_collides_V2i(rect, winput_mouse_pos())) {
        textbox_click(&ctx->tbox_search, &ctx->tbox_search_visual, winput_mouse_pos());
    }
    {
        strview_t focuser_widget_id = cstr_SL("tbox_search");
        if (Rect2i_collides_V2i(rect, winput_mouse_pos())) {
            focuser_take_focus(&ctx->focuser, focuser_widget_id);
        }
        Focusable btn_focus = focuser_make_focusable(&ctx->focuser, focuser_widget_id);
        if (btn_focus.has_focus) { b_draw_frame(rect, RED, 2); }
    }

    rect = divs[div_i++];
    b_draw_frame(rect, MAGENTA, 1);
    b_draw_texture(ctx->icon_search, Rect2i_center(rect, icon_size));
    {
        strview_t focuser_widget_id = cstr_SL("btn_search");
        if (Rect2i_collides_V2i(rect, winput_mouse_pos())) {
            focuser_take_focus(&ctx->focuser, focuser_widget_id);
        }
        Focusable btn_focus = focuser_make_focusable(&ctx->focuser, focuser_widget_id);
        if (btn_focus.has_focus) { b_draw_frame(rect, RED, 2); }
    }

    assert(div_i == countofi(divs));
}

typedef struct {
    int scroll_px;
    float vel_px;
} Simple_Scroll_t;

typedef struct {
    float scroll_px_float;
    float scroll_x0;
    float scroll_v0;
    float scroll_a0;
    float scroll_time;
    float scroll_acum_time;
} Complex_Scroll_t;

void ui_widget_bookmarks (Ctx *ctx, uitree_DrawInfo info) {
    if (kinput_key_held(GLFW_KEY_T)) { return; }
    b_draw_frame(info.area, BLUE, 1);
    int line_height = ui_line_height(ctx);

    enum ANYTYPE lilo = ANYTYPE_GET_ENUM(Simple_Scroll_t);
    printfd("got %d", lilo);

    Simple_Scroll_t *scroll = mapstrobj_get_default(&ctx->frame_objs, Simple_Scroll_t, (Simple_Scroll_t){0},
                            SC(&ctx->framearena, info.node_id, cstr_SL("_scroll")));
    mapstrobj_if_null_use_literal(scroll);
    int *scroll_px = &scroll->scroll_px;
    float *vel_px = &scroll->vel_px;

    Rect2i file_rect = {{ info.area.x, info.area.y, info.area.width, line_height }};

    ui_draw_text(ctx, cstr_SL("Places"), v2i(file_rect.x + CON_PAD, file_rect.y));
    file_rect.pos.y += line_height;

    ui__calculate_fancy_scroll_px(scroll_px, vel_px, info.area.height, ctx->bookmarks.size * file_rect.height, mice_wheel());
    Rect2i scissor_rect = {{info.area.x, file_rect.pos.y, info.area.width, info.area.height -  file_rect.height}};
    b_draw_begin_scissor(scissor_rect);

    if (vel_px != 0) { MUST_REDRAW = true; }
    file_rect.y += *scroll_px;
    for (dyna_foreach(File, iter, ctx->bookmarks)) {
        File *file = iter.ref;
        if (ui_button((Rect2i){.pos=v2i(file_rect.pos.x,file_rect.pos.y+1),.size=v2i(file_rect.width,file_rect.height-1)})) {
            printfd("Navigating to ["PRIstrw"]", PRIstrarg(File_get_path(*file)));
            add_location2(ctx, *file);
            refresh_listing(ctx);
        }
        Rect2i icon_rect = {{ file_rect.x + CON_PAD, file_rect.y, file_rect.height, file_rect.height }};
        b_draw_texture(ctx->icon_folder, icon_rect);
        V2i text_pos = {{ icon_rect.x + icon_rect.width + CON_PAD, file_rect.y }};
        ui_draw_text(ctx, File_get_bookmark_alias(*file), text_pos);
        file_rect.pos.y += line_height;
    }
    b_draw_end_scissor();

    if (*scroll_px) { // Draw separator.
        b_draw_rect((Rect2i){{scissor_rect.x,scissor_rect.y,scissor_rect.width,1}}, YELLOW);
    }
}

void ui_widget_explorer (Ctx *ctx, uitree_DrawInfo info) {
    enum { column_count = 3 };
    static float columns_drag_px[column_count-1] = { 280, 450 };
    static int is_dragging_idx = 0;
    static const strview_t column_names[column_count] = {
        cstr_SLc("Name"),
        cstr_SLc("Date"),
        cstr_SLc("Size"),
    };

    Rect2i area = info.area;
    int line_height = ui_line_height(ctx);
    int item_height = line_height + CON_PAD;
    b_draw_frame(area, BLUE, 1);
    Rect2i headers_rect = {{ area.x, area.y, area.width, item_height }};
    ui__calculate_multiple_drag_px(&is_dragging_idx, headers_rect, column_count-1, (float*)columns_drag_px, 30, area.width, CON_PAD*2, true);
    b_draw_frame(headers_rect, GREEN, 1);

    Rect2i col_rect[column_count] = { 0 };
    int x1 = area.x;
    V2i mouse = winput_mouse_pos();

    for (int i = 0; i < column_count; ++i) {
        int x2 = area.x + (i < column_count-1 ? (int)columns_drag_px[i] : area.width);
        col_rect[i] = (Rect2i) {{ x1, area.y, x2 - x1, area.height }};
        x1 += col_rect[i].width;
    }

    for (int i = 0; i < column_count-1; ++i) {
        b_draw_frame(col_rect[i], YELLOW, 1);
        ui_draw_text(ctx, column_names[i], col_rect[i].pos);
    }

    Rect2i scroll_rect = {{ area.x, area.y + item_height, area.width, area.height - item_height }};
    Complex_Scroll_t *scroll = mapstrobj_get(&ctx->frame_objs, Complex_Scroll_t,
                            SC(&ctx->framearena, info.node_id, cstr_SL("_scroll")));
    mapstrobj_if_null_use_literal(scroll);

    float *scroll_px_float = &scroll->scroll_px_float;
    {
        // @Note: Precise scrolling. Abstract me away in a struct please.
        float *scroll_x0 = &scroll->scroll_x0;
        float *scroll_v0 = &scroll->scroll_v0;
        float *scroll_a0 = &scroll->scroll_a0;
        float *scroll_time = &scroll->scroll_time;
        float *scroll_acum_time = &scroll->scroll_acum_time;
        bool focus = false;
        int focus_item_target = 0;
        if (kinput_key_pressed(GLFW_KEY_S)) { focus = true; focus_item_target = ctx->folder_files.size/2; }
        if (kinput_key_pressed(GLFW_KEY_D)) { focus = true; focus_item_target = ctx->folder_files.size; }
        ui__calculate_fancy_scroll_px_with_focus_animated(
            scroll_px_float, scroll_x0, scroll_v0, scroll_a0, scroll_time, scroll_acum_time,
            scroll_rect.height,
            (ctx->folder_files.size + 1) * item_height, mice_wheel() *2,
            focus, focus_item_target * item_height, item_height
        );
    }
    b_draw_frame(scroll_rect, RED, 1);
    int scroll_px_value = (int)*scroll_px_float;
    int *scroll_px = &scroll_px_value;


    // @Note(woy): This is to draw only the range of files which are visible:
    const int file_i_pad = 4;
    int file_i_start = (-*scroll_px) / item_height - file_i_pad;
    int file_i_end = file_i_start + scroll_rect.height / item_height + file_i_pad * 2;
    file_i_start = int_clamp(0, ctx->folder_files.size, file_i_start);
    file_i_end = int_clamp(0, ctx->folder_files.size, file_i_end);

    if (Rect2i_collides_V2i(scroll_rect, winput_mouse_pos())) {
        b_draw_begin_scissor(scroll_rect);
        // Mouse file hover.
        int row_i = file_i_start;
        for (int i = file_i_start; i < file_i_end; ++i, ++row_i) {
            File *file = &ctx->folder_files.items[i];
            Rect2i file_select_rect = {{ scroll_rect.x, scroll_rect.y + item_height * row_i + *scroll_px,
                scroll_rect.width, item_height-1
            }};
            if (Rect2i_collides_V2i(file_select_rect, winput_mouse_pos())) {
                b_draw_rect(file_select_rect, BLUE);
                if (mice_double_click()) {
                    mice_consume(MouseLeft);
                    printfd("DOUBLE CLICK on file %d", i);
                    add_location2(ctx, *file);
                    refresh_listing(ctx);
                    break;
                }
                else if (mice_pressed(MouseLeft)) {
                    mice_consume(MouseLeft);
                    printfd("Normal click on file %d", i);
                    break;
                }
            }
        }
        b_draw_end_scissor();
    }

    {
        int col_i = 0; // name
        int row_i = file_i_start;
        Rect2i column = col_rect[col_i];
        column.height -= item_height; column.y += item_height;
        b_draw_begin_scissor(column);
        for (int i = file_i_start; i < file_i_end; ++i) {
            File *file = &ctx->folder_files.items[i];
            ++row_i;
            V2i pos = {{ col_rect[col_i].pos.x, col_rect[col_i].pos.y + item_height * row_i + *scroll_px }};
            Rect2i icon_rect = {{ pos.x, pos.y - CON_PAD, item_height, item_height }};
            pos.x += item_height;
            Image icon = file->is_dir ? ctx->icon_folder : ctx->icon_file;
            b_draw_texture(icon, icon_rect);
            ui_draw_text(ctx, strpool_get(file->strpool, file->bookmark_alias), pos);
        }
        b_draw_end_scissor();
    }

    {
        int col_i = 1; // date
        int row_i = file_i_start;
        Rect2i column = col_rect[col_i];
        column.height -= item_height; column.y += item_height;
        b_draw_begin_scissor(column);
        for (int i = file_i_start; i < file_i_end; ++i) {
            File *file = &ctx->folder_files.items[i];
            ++row_i;
            V2i pos = {{ col_rect[col_i].pos.x, col_rect[col_i].pos.y + item_height * row_i + *scroll_px }};
            Arena arena = ctx->framearena;
            ui_draw_text(ctx, SF(&arena, "%02d/%02d/%04d %02d:%02d",
                file->mod_date.tm_mday,
                file->mod_date.tm_mon,
                file->mod_date.tm_year + 1900,
                file->mod_date.tm_hour,
                file->mod_date.tm_min
                ),
                pos
            );
        }
        b_draw_end_scissor();
    }
}



void widget_draw__main(Ctx *ctx, uitree_NewDrawInfo info) {
    Arena *perm = &ctx->framearena;
    Uitree *tree = arena_new(perm, Uitree, 1);
    uitree_create(tree, perm);

    UILayoutNode root = make_widget();
    root.area = info.area;
    widget_set_id(tree, &root, cstr_SL("Root"));

    {
        UILayoutNode vsplit = make_widget(); {
            widget_set_id(tree, &vsplit, cstr_SL("main_vsplit"));
            static Widget_2split_State vsplit_state = { .size=-0.2f,.is_percentage_or_px=0,.gap=2,.pad=4 };
            widget_set_state(&vsplit, Anytype_make(vsplit_state, Widget_2split_State));
            widget_set_container_function(&vsplit, widget_vsplit_new);
            widget_set_drawing_func(&vsplit, UI_WIDGET_VSPLIT_DRAG);
        }
        UILayoutNode header = make_widget(); {
            widget_set_id(tree, &header, cstr_SL("header"));
            widget_set_drawing_func(&header, UI_WIDGET_BOOKMARKS);
        }
        UILayoutNode explorer = make_widget(); {
            widget_set_id(tree, &explorer, cstr_SL("explorer"));
            widget_set_drawing_func(&explorer, UI_WIDGET_EXPLORER);
        }
        widget_add_child(tree, &vsplit, header);
        widget_add_child(tree, &vsplit, explorer);
        widget_add_child(tree, &root, vsplit);
    }
    uitree_build_end_new(tree, root);

    //b_draw_rect((Rect2i) {.size=ctx->window_size}, DEFAULT_BG); // Background.
    b_draw_rect((Rect2i) {.size=ctx->window_size}, DARKPURPLE); // Background.

    int i = -1;
    uitree_List_DrawInfo_It it = { 0 };
    while(uitree_List_DrawInfo_it_next(&tree->out_draw_list, &it)) {
        ++i;
        int depth = tree->out_draw_list.size - i;
        uitree_DrawInfo draw = *it.item;
        dbuf_set_layer((uint8_t)depth);
        widget_func[draw.user_draw_func_id](ctx, draw);
        printfd("Got node ("PRIstrw"), area "Rect2i_Fmt, PRIstrarg(draw.node_id), Rect2i_Arg(draw.area));
    }
    printfd("End");

}

void draw_all(Ctx *ctx, const bool force_redraw) {
    static int draw_count = 0;
    ++draw_count;
    dbuf_draw_start();
    mapstrobj_set_frame(&ctx->frame_objs, draw_count);

    uitree_NewDrawInfo info = {
        .node_path = cstr_SL("root"),
        .area = (Rect2i){ .size=ctx->window_size },
        .state = ANYTYPE_NULL,
    };
    widget_draw__main(ctx, info);

    static int redraw_count = 0;
    if (force_redraw || dbuf_do_buffers_differ()) {
        ++redraw_count;
        //draw_rect((Rect2i) {.size=ctx->window_size}, BLACK);
        //printfd(ANSI_MAG"Must redraw. "Bool_Fmt"!!!!!!1", Bool_Arg(dbuf_do_buffers_differ()));
        dbuf_draw_end();
        //drawbuf_swap();
        // TODO NONE OF THIS SHOULD BE HERE.
        draw_rect((Rect2i){.pos=v2i(ctx->window_size.x-30,ctx->window_size.y-30),.size=v2i(25,24)}, BLACK);
        draw_text(SF(&ctx->framearena, "%d", redraw_count), ctx->font1, v2i(ctx->window_size.x-30,ctx->window_size.y-30), 10, 0, 0, GREEN);
        rgba_to_bgra((u32*)x11_get_buffer(), (Rect2i){.size=ctx->window_size}, ctx->window_size.x);
        char *buffer = x11_swap_buffer();
        wod_set_buffer((u32*)buffer, ctx->window_size, ctx->window_size.x);
    }

    mapstrobj_free_older_than_frame(&ctx->frame_objs, ctx->framearena);
}


//void draw_all2(Ctx *ctx, const bool force_redraw) {

    //static int redraw_count = 0;
    //static Uitree tree = { 0 };
    //static Uitree *t = &tree;
    //static bool setup = false;
    //if (!setup) { setup = true; uitree_create(t); }


    ////Anytype any = {
        ////.data = &tree,
        ////.size = sizeofi(tree),
        ////.user_type = 10,
    ////};

    ////Anytype any = Anytype_make(tree, 11);

    ////Uitree *mytree = Anytype_read_as(any, 11, Uitree);

    //uitree_ContainerFunc *delme_container_func;
    //delme_container_func = (uitree_ContainerFunc*)&widget_hsplit_all_void;
    ////delme_container_func = (uitree_ContainerFunc*)& ui_widget_vsplit_drag;


    
    //UILayoutNode root = { 0 };

    //{
        //UILayoutNode widget_explorer = {
            //.user_draw_func_id = UI_WIDGET_EXPLORER,
        //};

        //{
            //UILayoutNode vsplit = {
                //.container_func = widget_vsplit,
            //};
            //Widget_2split_State *vsplit_state = arena_new(&t->arena, Widget_2split_State, 1);
            //*vsplit_state = (Widget_2split_State) {
                //.size = 50,
                //.is_percentage_or_px = true
            //};
            //vsplit.is_arena_or_map = true;
            //vsplit.state_from_arena = make_blob(*vsplit_state);

            //{
                //UILayoutNode vsplit_drag = {
                    //.container_func = widget_vsplit,
                    //.user_draw_func_id = UI_WIDGET_VSPLIT_DRAG,
                    //.identifier = strpool_append(&t->strpool, cstr_SL("VsplitDrag"))
                //};
                //Widget_2split_State *vsplit_drag_state = arena_new(&t->arena, Widget_2split_State, 1);
                //*vsplit_drag_state = (Widget_2split_State) {
                    //.size = 50,
                    //.is_percentage_or_px = true
                //};
                //vsplit.is_arena_or_map = false;
                //mapstrobj_upsert(&ctx->frame_objs, cstr_SL("VsplitDrag"), blob_arg(*vsplit_drag_state));

                //uilayoutnode_add_child(t, &vsplit_drag, widget_explorer);
                //uilayoutnode_add_child(t, &vsplit_drag, widget_explorer);

                //uilayoutnode_add_child(t, &vsplit, widget_explorer);
                //uilayoutnode_add_child(t, &vsplit, vsplit_drag);
            //}

            //uilayoutnode_add_child(t, &root, vsplit);
        //}
    //}

    //// Calculate layout and stuff.

    //for (int i = 0; i < root.children.size; ++i) {
    //}



    //// uitree_set_id_prefix(cstr_SL("main_view"));

    //{
        //uitree_build_start(t, (Rect2i){ .size=ctx->window_size });
        //uitree_Node widget;
        //uitree_Node con_tree = uitree_container_dumb(widget_stack);
        //{
            //uitree_Node con_vsplit = uitree_container_dumb(widget_vsplit);
            //widget_2split_set_user_default_state(t, &con_vsplit, ui_line_height(ctx) * 2, true, 2, 1);
            //{
                //widget = uitree_widget_id(t, UI_WIDGET_HEADER, cstr_SL("header"));
                //uitree_container_add_child(t, &con_vsplit, widget);
            //}
            //{
                //uitree_Node con_hsplit = uitree_container(t, cstr_SL("hsplit"), widget_hsplit, UI_WIDGET_HSPLIT_DRAG);
                //widget_2split_set_user_default_state(t, &con_hsplit, ui_line_height(ctx) * 8, true, CON_PAD*2, 1);
                //{
                    //{
                        //widget = uitree_widget_id(t, UI_WIDGET_BOOKMARKS, cstr_SL("bookmarks"));
                        //uitree_container_add_child(t, &con_hsplit, widget);
                    //}
                    //{
                        //widget = uitree_widget_id(t, UI_WIDGET_EXPLORER, cstr_SL("explorer"));
                        //// uitree_widget_set_static_state(t, &widget, ctx->static_states.explorer_state);
                        //uitree_container_add_child(t, &con_hsplit, widget);
                    //}
                //}
                //uitree_container_add_child(t, &con_vsplit, con_hsplit);
            //}
            //uitree_container_add_child(t, &con_tree, con_vsplit);
        //}
        //t->root_node = con_tree;
        //uitree_build_end(t);
    //}

    //dbuf_draw_start();
    //b_draw_rect((Rect2i) {.size=ctx->window_size}, DEFAULT_BG); // Background.
    ////b_draw_rect((Rect2i) {.size=ctx->window_size}, DARKPURPLE); // Background.

    //int i = -1;
    //uitree_List_DrawInfo_It it = { 0 };
    //while(uitree_List_DrawInfo_it_next(&t->out_draw_list, &it)) {
        //++i;
        //int depth = t->out_draw_list.size - i;
        //uitree_DrawInfo draw = *it.item;
        //dbuf_set_layer((uint8_t)depth);
        //widget_func[draw.user_draw_func_id](ctx, draw);
    //}

    ////b_draw_text(ctx->font1, cstr_SL("The quick brown fox jumps over the lazy dog éjpyóç"), v2i(0,300), ctx->font1.font_size, MAGENTA);
    ////b_draw_text(ctx->font1, SF(&ctx->framearena, "Hello %d", 10), v2i(0,200), ctx->font1.font_size, MAGENTA);
    ////b_draw_text(ctx->font1, SC(&ctx->framearena, SF(&ctx->framearena, "%d:%f", 10, 3.4f)), v2i(0,100), ctx->font1.font_size, MAGENTA);

    //if (force_redraw || dbuf_do_buffers_differ()) {
        //++redraw_count;
        ////draw_rect((Rect2i) {.size=ctx->window_size}, BLACK);
        ////printfd(ANSI_MAG"Must redraw. "Bool_Fmt"!!!!!!1", Bool_Arg(dbuf_do_buffers_differ()));
        //dbuf_draw_end();
        ////drawbuf_swap();
        //// TODO NONE OF THIS SHOULD BE HERE.
        //draw_rect((Rect2i){.pos=v2i(ctx->window_size.x-30,ctx->window_size.y-30),.size=v2i(25,24)}, BLACK);
        //draw_text(SF(&ctx->framearena, "%d", redraw_count), ctx->font1, v2i(ctx->window_size.x-30,ctx->window_size.y-30), 10, 0, 0, GREEN);
        //rgba_to_bgra((u32*)x11_get_buffer(), (Rect2i){.size=ctx->window_size}, ctx->window_size.x);
        //char *buffer = x11_swap_buffer();
        //wod_set_buffer((u32*)buffer, ctx->window_size, ctx->window_size.x);
    //}

    ////printfd("Arena consumption is "PRIbyte" out of "PRIbyte, PRIbytearg((1 << 20) - (t->arena.end - t->arena.beg)), PRIbytearg(1 << 20));
//}

#endif
