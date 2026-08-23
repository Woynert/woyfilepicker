#ifndef UI_COMMON_H
#define UI_COMMON_H

#include "drawbuffer.h"
#include "state.h"
#include "uitree.h"
#include "ui_mouse_input.h"


void ui__calculate_fancy_scroll_px(
    int *scroll_px, float *vel_px, int container_px,
    int child_px, int force_up_or_down
) {
    enum { IMPULSE=40 };
    if (force_up_or_down != 0) {
        *vel_px = (float)(force_up_or_down * IMPULSE);
    }
    if (*vel_px != 0) {
        *scroll_px += (int)*vel_px;
        *vel_px *= 0.5f;
        if (fabsf(*vel_px) < 0.1f) {
            *vel_px = 0;
        }
    }
    int max_scroll = int_max(container_px, child_px) - container_px;
    *scroll_px = int_clamp(-max_scroll, 0, *scroll_px);
}


/// @Param x_or_y: True is X (Horizontal), False is Y (Vertical).
/// @Param minimum_gap_px: Minimum gap between drags.
/// @Param maximum_px: Maximum value a gap can go.
/// @Param handle_gap_px: width or height of gap for the mouse to collide with.
void ui__calculate_multiple_drag_px(
        int *is_dragging_handle_idx, Rect2i area,
        int drag_count, float* drag_progress_px,
        int minimum_gap_px, int maximum_px, int handle_gap_px, bool x_or_y
) {
    // 0 -> Not dragging, [1 ... drag_count ] -> Dragging.
    bool is_dragging = int_in_range_inclusive(0, drag_count-1, *is_dragging_handle_idx-1);
    if (!is_dragging) {
        for (int i = 0; i < drag_count; ++i) {
            int progress_px = (int)drag_progress_px[i];
            Rect2i drag_area = x_or_y ? (Rect2i){{
                area.x + progress_px - handle_gap_px/2, area.y,
                handle_gap_px, area.height
            }} : (Rect2i){{
                area.y, area.y + progress_px - handle_gap_px/2,
                area.width, handle_gap_px
            }};
            if (Rect2i_collides_V2i(drag_area, winput_mouse_pos())) {
                b_draw_rect(drag_area, YELLOW);
                if (mice_pressed(MouseLeft)) {
                    mice_consume(MouseLeft);
                    is_dragging = true;
                    *is_dragging_handle_idx = i+1;
                    break;
                }
            }
        }
    }
    if (is_dragging) {
        int drag_id = *is_dragging_handle_idx-1; // Because zero is reserved.
        int min_px = minimum_gap_px + (drag_id <= 0 ? 0 : (int)drag_progress_px[drag_id -1]);
        int max_px = - minimum_gap_px + (drag_id >= drag_count-1 ? maximum_px : (int)drag_progress_px[drag_id +1]);
        int mouse_px = - handle_gap_px/2 + (x_or_y ? winput_mouse_pos().x : winput_mouse_pos().y);
        drag_progress_px[drag_id] = (float)int_clamp(min_px, max_px, mouse_px - (x_or_y ? area.x : area.y));
    }
    if (mice_released(MouseLeft)) {
        *is_dragging_handle_idx = 0;
    }
}

/// @Note see ui__calculate_multiple_drag_px for param info.
void ui__calculate_multiple_drag_percent(
        int *is_dragging_handle_idx, Rect2i area,
        int drag_count, float* drag_progress,
        float minimum_gap_percent, int maximum_px, int handle_gap_px, bool x_or_y
) {
    // 0 -> Not dragging, [1 ... drag_count ] -> Dragging.
    bool is_dragging = int_in_range_inclusive(0, drag_count-1, *is_dragging_handle_idx-1);
    if (!is_dragging && mice_pressed(MouseLeft)) {
        for (int i = 0; i < drag_count; ++i) {
            float progress = drag_progress[i];
            Rect2i drag_area = x_or_y ? (Rect2i){{
                area.x + (int)(progress * (float)area.width) - handle_gap_px/2, area.y,
                handle_gap_px, area.height
            }} : (Rect2i){{
                area.y, area.y + (int)(progress * (float)area.height) - handle_gap_px/2,
                area.width, handle_gap_px
            }};
            if (Rect2i_collides_V2i(drag_area, winput_mouse_pos())) {
                is_dragging = true;
                *is_dragging_handle_idx = i+1;
                break;
            }
        }
    }
    if (is_dragging) {
        int drag_id = *is_dragging_handle_idx-1; // Because zero is reserved.
        float area_start = (float)(x_or_y ? area.x : area.y);
        float area_length = (float)(x_or_y ? area.width : area.height);
        float maximum_percent = (float)maximum_px/area_length;
        float min_perc = minimum_gap_percent + ((drag_id <= 0) ? 0 : drag_progress[drag_id -1]);
        float max_perc = (drag_id >= drag_count-1) ? maximum_percent : drag_progress[drag_id +1] - minimum_gap_percent;
        float mouse_px = (float)(x_or_y ? winput_mouse_pos().x : winput_mouse_pos().y);
        drag_progress[drag_id] = float_clamp(min_perc, max_perc, (mouse_px - area_start)/area_length);
    }
    if (mice_released(MouseLeft)) {
        *is_dragging_handle_idx = 0;
    }
}

/*
void ui__calculate_multiple_draw(
        int *is_dragging_handle_idx, Rect2i area, int drag_count,
        MultipleDrag* drags, int handle_gap_px, bool x_or_y
) {
    // 0 -> Not dragging, [1 ... drag_count ] -> Dragging.
    enum { NOT_DRAGGING };
    bool is_dragging = int_in_range_inclusive(1, drag_count, *is_dragging_handle_idx);
    if (!is_dragging && mice_pressed(MouseLeft)) {
        for (int i = 0; i < drag_count; ++i) {
            MultipleDrag* drag = &drags[i];
            Rect2i drag_area = x_or_y ? (Rect2i){{
                area.x + drag->progress_px - handle_gap_px/2, area.y,
                handle_gap_px, area.height
            }} : (Rect2i){{
                area.y, area.y + drag->progress_px - handle_gap_px/2,
                area.width, handle_gap_px
            }};
            if (Rect2i_collides_V2i(drag_area, winput_mouse_pos())) {
                is_dragging = true;
                *is_dragging_handle_idx = i+1;
                break;
            }
        }
    }
    if (is_dragging) {
        int mouse_pos = x_or_y ? winput_mouse_pos().x : winput_mouse_pos().y;
        MultipleDrag* drag = &drags[*is_dragging_handle_idx];
        drag->progress_px = int_clamp(drag->min_px, drag->max_px, mouse_pos - (x_or_y ? area.x : area.y));
    }
    if (mice_released(MouseLeft)) {
        *is_dragging_handle_idx = NOT_DRAGGING;
    }
}
   */

void widget_stack(Rect2i area, int child_count, Rect2i *children, void *user_ctx, uitree_WidgetState *state) {
    (void)user_ctx, (void)state;
    for (int i = 0; i < child_count; ++i) { children[i] = area; }
}


void widget_vlist(Rect2i area, int child_count, Rect2i *children, void *user_ctx, uitree_WidgetState *state) {
    (void)user_ctx, (void)state;
    for (int i = 0; i < child_count; ++i) {
        Rect2i *child = &children[i];
        child->width = area.width;
        child->x = area.x;
        child->height = area.height / child_count;
        child->y = area.y + child->height * i;
    }
}

typedef struct widget_2split_state_t {
    float *size;
    int *is_percentage_or_px;
    int *is_dragging;
    int *pad;                // px
    int *gap;                // px
    Rect2i *drag_area;
} widget_2split_state_t;

widget_2split_state_t widget_2split_get_state(uitree_WidgetState *state) {
    return (widget_2split_state_t) {
        .size                = &state->float_a,
        .is_percentage_or_px = &state->int_a,
        .is_dragging         = &state->int_b,
        .pad                 = &state->int_c,
        .gap                 = &state->int_d,
        .drag_area           = &state->rect_a,
    };
}

void widget_2split_set_user_default_state(Uitree *tree, uitree_Node *node, int size, bool is_percentage_or_px, int gap_px, int pad_px) {
    uitree_WidgetState *state = arena_new(&tree->arena, uitree_WidgetState, 1);
    if (state == NULL) { return; }
    widget_2split_state_t vars = widget_2split_get_state(state);

    *vars.pad = pad_px;
    *vars.gap = gap_px;
    *vars.is_percentage_or_px = is_percentage_or_px;
    if (is_percentage_or_px == 0) {
        *vars.size = (float)(size - 50) / 100.f; // %
    } else {
        *vars.size = (float)size + (float)gap_px; // px
    }
    node->user_default_state = state;
}

void widget_vsplit(Rect2i area, int child_count, Rect2i *children, void *user_ctx, uitree_WidgetState *state) {
    (void)user_ctx;
    const widget_2split_state_t vars = widget_2split_get_state(state);
    int first_child_height;
    area = Rect2i_add_padding_all(area, *vars.pad);
    // Determines whether the sizes are percentages (relative) or pixels (absolute).
    if (*vars.is_percentage_or_px == 0) {
        first_child_height = (int)((float)area.height * (*vars.size + 0.5f));
    } else {
        first_child_height = (int)*vars.size;
    }
    if (child_count > 0) {
        Rect2i *child = &children[0];
        child->width = area.width;
        child->x = area.x;
        child->height = first_child_height - (*vars.gap/2);
        child->y = area.y;
    }
    if (child_count > 1) {
        Rect2i *child = &children[1];
        child->width = area.width;
        child->x = area.x;
        child->height = area.height - children[0].height - *vars.gap;
        child->y = area.y + children[0].height + *vars.gap;
    }
    // This rect will be used for dragging.
    state->rect_a = (Rect2i) {
        .x      = children[0].x,
        .width  = children[0].width,
        .y      = children[0].y + children[0].height,
        .height = *vars.gap,
    };
    if (child_count > 2) { printfd("WAR: Too many children."); }
}

void widget_hsplit(Rect2i area, int child_count, Rect2i *children, void *user_ctx, uitree_WidgetState *state) {
    (void)user_ctx;
    const widget_2split_state_t vars = widget_2split_get_state(state);
    area = Rect2i_add_padding_all(area, *vars.pad);
    // Determines whether the sizes are percentages (relative) or pixels (absolute).
    int first_child_width;
    if (*vars.is_percentage_or_px == 0) {
        first_child_width = (int)((float)area.width * (*vars.size + 0.5f));
    } else {
        first_child_width = (int)*vars.size;
    }
    if (child_count > 0) {
        Rect2i *child = &children[0];
        child->height = area.height;
        child->x = area.x;
        child->width = first_child_width - (*vars.gap/2);
        child->y = area.y;
    }
    if (child_count > 1) {
        Rect2i *child = &children[1];
        child->height = area.height;
        child->y = area.y;
        child->width = area.width - children[0].width - *vars.gap;
        child->x = area.x + children[0].width + *vars.gap;
    }
    // This rect will be used for dragging.
    state->rect_a = (Rect2i) {
        .y      = children[0].y,
        .height = children[0].height,
        .x      = children[0].x + children[0].width,
        .width  = *vars.gap,
    };
    if (child_count > 2) { printfd("WAR: Too many children."); }
}

void ui_widget_vsplit_drag(Ctx *ctx, uitree_DrawInfo info) {
    (void)ctx;
    Rect2i area = info.area;
    b_draw_frame(area, ORANGE, 1);
    widget_2split_state_t vars = widget_2split_get_state(info.state);
    if (mice_in_rect(*vars.drag_area) || *vars.is_dragging) {
        b_draw_rect(*vars.drag_area, ORANGE);
        if (mice_pressed(MouseLeft)) {
            mice_consume(MouseLeft);
            *vars.is_dragging = true;
        }
    }
    if (*vars.is_dragging) {
        int mouse_y = winput_mouse_pos().y;
        if (*vars.is_percentage_or_px == 0) {
            *vars.size = ((float)mouse_y - ((float)area.y + (float)area.height / 2.0f)) / (float)area.height;
            *vars.size = float_clamp(-0.45f, 0.45f, *vars.size);
        } else {
            float factor = ((float)mouse_y - (float)area.y) / (float)area.height;
            *vars.size = factor * (float)area.height;
            *vars.size = (float)int_clamp(30, area.height-30, (int)*vars.size);
        }
        // ↑↑↑ This ensures at least a % is visible at minimum.
        if (mice_released(MouseLeft)) { *vars.is_dragging = false; }
    }
}

void ui_widget_hsplit_drag(Ctx *ctx, uitree_DrawInfo info) {
    /*
typedef struct widget_2split_state_t {
    float *size;
    int *is_percentage_or_px;
    int *is_dragging;
    int *pad;                // px
    int *gap;                // px
    Rect2i *drag_area;
} widget_2split_state_t;
       */
    (void)ctx;
    Rect2i area = info.area;
    widget_2split_state_t vars = widget_2split_get_state(info.state);
    if (*vars.is_percentage_or_px) {
        //ui__cal
        ui__calculate_multiple_drag_px(
            vars.is_dragging, info.area, 1, vars.size, 30, info.area.width, *vars.gap, true);
    } else {
        //ui__calculate_multiple_drag_px(
            //vars.is_dragging, info.area, 1, vars.size, *vars.gap, info.area.width, *vars.gap * 2, true);
    }
    //ui__calculate_multiple_drag_percent

}

/*
void ui_widget_hsplit_drag(Ctx *ctx, uitree_DrawInfo info) {
    (void)ctx;
    Rect2i area = info.area;
    widget_2split_state_t vars = widget_2split_get_state(info.state);
    if (mice_in_rect(*vars.drag_area) || *vars.is_dragging) {
        b_draw_rect(*vars.drag_area, ORANGE);
        if (mice_pressed(MouseLeft)) {
            mice_consume(MouseLeft);
            *vars.is_dragging = true;
        }
    }
    if (*vars.is_dragging) {
        int mouse_x = winput_mouse_pos().x;
        if (*vars.is_percentage_or_px == 0) {
            *vars.size = ((float)mouse_x - ((float)area.x + (float)area.width / 2.0f)) / (float)area.width;
            *vars.size = float_clamp(-0.45f, 0.45f, *vars.size);
        } else {
            float factor = ((float)mouse_x - (float)area.x) / (float)area.width;
            *vars.size = factor * (float)area.width;
            *vars.size = (float)int_clamp(30, area.width-30, (int)*vars.size);
        }
        // ↑↑↑ This ensures at least a % is visible at minimum.
        if (mice_released(MouseLeft)) { *vars.is_dragging = false; }
    }
}
   */

typedef struct widget_3split_state_t {
    int *is_setup;
    int *is_dragging;
    float *percentage1;
    float *percentage2;
    Rect2i *drag_area1;
    Rect2i *drag_area2;
} widget_3split_state_t;

widget_3split_state_t widget_3split_get_state(uitree_WidgetState *state) {
    return (widget_3split_state_t) {
        .is_setup = &state->int_a,
        .is_dragging = &state->int_b,
        .percentage1 = &state->float_a,
        .percentage2 = &state->float_b,
        .drag_area1 = &state->rect_a,
        .drag_area2 = &state->rect_b,
    };
}

void widget_3hsplit_set_user_default_state(Uitree *tree, uitree_Node *node, int p_percentage1, int p_percentage2) {
    uitree_WidgetState *state = arena_new(&tree->arena, uitree_WidgetState, 1);
    if (state == NULL) { return; }
    widget_3split_state_t vars = widget_3split_get_state(state);
    node->user_default_state = state;
    *vars.is_setup = true;
    *vars.percentage1 = (float)p_percentage1/100.f;
    *vars.percentage2 = (float)p_percentage2/100.f;
}

void widget_3hsplit(Rect2i area, int child_count, Rect2i *children, void *user_ctx, uitree_WidgetState *state) {
    (void)user_ctx;
    const int pad = 2;
    const widget_3split_state_t vars = widget_3split_get_state(state);
    if (child_count > 0) {
        Rect2i *child = &children[0];
        child->height = area.height;
        child->y = area.y;
        child->width = (int)((float)area.width * *vars.percentage1);
        child->x = area.x;
    }
    if (child_count > 1) {
        Rect2i *child = &children[1];
        child->height = area.height;
        child->y = area.y;
        child->width = (int)((float)area.width * *vars.percentage2) - children[0].width;
        child->x = children[0].x + children[0].width;
    }
    if (child_count > 2) {
        Rect2i *child = &children[2];
        child->height = area.height;
        child->y = area.y;
        child->width = area.width - children[0].width - children[1].width;
        child->x = children[1].x + children[1].width;
    }
    children[0] = Rect2i_add_padding_all(children[0], pad);
    children[1] = Rect2i_add_padding_all(children[1], pad);
    children[2] = Rect2i_add_padding_all(children[2], pad);
    *vars.drag_area1 = (Rect2i) {
        .y      = children[0].y,
        .height = children[0].height,
        .x      = children[0].x + children[0].width,
        .width = pad * 2,
    };
    *vars.drag_area2 = (Rect2i) {
        .y      = children[0].y,
        .height = children[0].height,
        .x      = children[1].x + children[1].width,
        .width = pad * 2,
    };
    if (child_count > 3) { printfd("WAR: Too many children."); }
}

void ui_widget_3hsplit_drag(Ctx *ctx, uitree_DrawInfo info) {

    enum { NOT_DRAGGING, IS_DRAGGING_1ND, IS_DRAGGING_2ND };

    const float PAD = 0.02f;
    Rect2i area = info.area;
    widget_3split_state_t vars = widget_3split_get_state(info.state);

    if (!*vars.is_setup) {
        *vars.is_setup = true;
        *vars.percentage1 = 0.33f;
        *vars.percentage2 = 0.66f;
    }

    //if (*vars.percentage2 == 0) { *vars.percentage2 = 50; }

    if (mice_in_rect(*vars.drag_area1) || (*vars.is_dragging == IS_DRAGGING_1ND)) {
        b_draw_rect(*vars.drag_area1, BLUE);
        if (mice_pressed_consume(MouseLeft)) {
            *vars.is_dragging = IS_DRAGGING_1ND;
        }
    }

    else if (mice_in_rect(*vars.drag_area2) || (*vars.is_dragging == IS_DRAGGING_2ND)) {
        b_draw_rect(*vars.drag_area2, RED);
        if (mice_pressed_consume(MouseLeft)) {
            *vars.is_dragging = IS_DRAGGING_2ND;
        }
    }

    if (*vars.is_dragging == IS_DRAGGING_1ND) {
        int mouse_x = winput_mouse_pos().x;
        *vars.percentage1 = ((float)mouse_x - (float)area.x) / (float)area.width;
        *vars.percentage1 = float_clamp(PAD, *vars.percentage2 - PAD, *vars.percentage1);
    }
    if (*vars.is_dragging == IS_DRAGGING_2ND) {
        int mouse_x = winput_mouse_pos().x;
        *vars.percentage2 = ((float)mouse_x - (float)area.x) / (float)area.width;
        *vars.percentage2 = float_clamp(*vars.percentage1 + PAD, 1.f - PAD, *vars.percentage2);
    }
    if (*vars.is_dragging != NOT_DRAGGING && mice_released(MouseLeft)) {
        *vars.is_dragging = NOT_DRAGGING;
    }
}

#endif

