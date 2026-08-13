#ifndef UI_COMMON_H
#define UI_COMMON_H

#include "raylib_drawbuffer.h"
#include "state.h"
#include "uitree.h"
#include "ui_mouse_input.h"

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

typedef struct widget_vsplit_state_t {
    float *size;
    int *is_percentage_or_px;
    int *is_dragging;
    int *pad;                // px
    int *gap;                // px
    Rect2i *drag_area;
} widget_vsplit_state_t;

widget_vsplit_state_t widget_vsplit_get_state(uitree_WidgetState *state) {
    return (widget_vsplit_state_t) {
        .size                = &state->float_a,
        .is_percentage_or_px = &state->int_a,
        .is_dragging         = &state->int_b,
        .pad                 = &state->int_c,
        .gap                 = &state->int_d,
        .drag_area           = &state->rect_a,
    };
}

void widget_vsplit_set_user_default_state(Uitree *tree, uitree_Node *node, int size, bool is_percentage_or_px, int gap_px, int pad_px) {
    uitree_WidgetState *state = arena_new(&tree->arena, uitree_WidgetState, 1);
    if (state == NULL) { return; }
    widget_vsplit_state_t vars = widget_vsplit_get_state(state);

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
    const widget_vsplit_state_t vars = widget_vsplit_get_state(state);
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

void ui_widget_vsplit_drag(Ctx *ctx, uitree_DrawInfo info) {
    (void)ctx;
    Rect2i area = info.area;
    widget_vsplit_state_t vars = widget_vsplit_get_state(info.state);

    if (mice_in_rect(*vars.drag_area) || *vars.is_dragging) {
        b_DrawRect(*vars.drag_area, ORANGE);
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

        if (mice_released(MouseLeft)) {
            *vars.is_dragging = false;
        }
    }
}


typedef struct widget_3hsplit_state_t {
    int *is_setup;
    int *is_dragging;
    float *percentage1;
    float *percentage2;
    Rect2i *drag_area1;
    Rect2i *drag_area2;
} widget_3hsplit_state_t;

widget_3hsplit_state_t widget_3hsplit_get_state(uitree_WidgetState *state) {
    return (widget_3hsplit_state_t) {
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
    widget_3hsplit_state_t vars = widget_3hsplit_get_state(state);
    node->user_default_state = state;
    *vars.is_setup = true;
    *vars.percentage1 = (float)p_percentage1/100.f;
    *vars.percentage2 = (float)p_percentage2/100.f;
}

void widget_3hsplit(Rect2i area, int child_count, Rect2i *children, void *user_ctx, uitree_WidgetState *state) {
    (void)user_ctx;
    const int pad = 2;
    const widget_3hsplit_state_t vars = widget_3hsplit_get_state(state);
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
    widget_3hsplit_state_t vars = widget_3hsplit_get_state(info.state);

    if (!*vars.is_setup) {
        *vars.is_setup = true;
        *vars.percentage1 = 0.33f;
        *vars.percentage2 = 0.66f;
    }

    if (*vars.percentage2 == 0) { *vars.percentage2 = 50; }

    if (mice_in_rect(*vars.drag_area1) || (*vars.is_dragging == IS_DRAGGING_1ND)) {
        b_DrawRect(*vars.drag_area1, BLUE);
        if (mice_pressed_consume(MouseLeft)) {
            *vars.is_dragging = IS_DRAGGING_1ND;
        }
    }

    else if (mice_in_rect(*vars.drag_area2) || (*vars.is_dragging == IS_DRAGGING_2ND)) {
        b_DrawRect(*vars.drag_area2, RED);
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

