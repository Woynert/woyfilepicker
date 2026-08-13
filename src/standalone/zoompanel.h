#ifndef ZOOMPANEL_H
#define ZOOMPANEL_H

#include "portable_utils.h"
#include "raylib_extra.h"
#include "ui_mouse_input.h"
#include "winput.h"


enum ZOOMPANEL_CONF {
    ZOOMPANEL_CONF_PROPORTIONAL,
    ZOOMPANEL_CONF_PIXEL_PERFECT,
};
#define ZOOMPANEL_CONF_DEFAULT ZOOMPANEL_CONF_PROPORTIONAL


typedef struct Zoompanel {
    // Public

    V2i offset_from_origin;
    float zoom; // Default 1: 1 -> 100%, 0.5 -> 50%, 2 -> 200%.
    enum ZOOMPANEL_CONF config;
    WinputMice button;

    // Private

    int zoom_level;

    V2i original_size;
    V2i trasformed_size;

    bool is_dragging;
    V2i drag_start;
    V2i drag_bk;
} Zoompanel;


void zoompanel_init(Zoompanel *panel, enum ZOOMPANEL_CONF config, WinputMice button) {
    *panel = (Zoompanel) { 0 };
    panel->zoom = 1;
    panel->config = config;
    panel->button = button;
}


void zoompanel__panning(Zoompanel *panel, Rect2i draw_area) {
    V2i mouse = GetMousePositioni();

    if (CheckCollisionPointReci(mouse, draw_area)) {
        if (winput_mice_pressed(panel->button)) {
            if (!panel->is_dragging) {
                panel->is_dragging = true;
                panel->drag_start = mouse;
                panel->drag_bk = panel->offset_from_origin;
            }
        }
    }
    if (winput_mice_released(panel->button)) {
        panel->is_dragging = false;
    }

    if (panel->is_dragging) {
        panel->offset_from_origin = v2i_add(v2i_sub(mouse, panel->drag_start), panel->drag_bk);
    }
}


void zoompanel__zooming(Zoompanel *panel, Rect2i draw_area) {

    int MIN_ZOOM_LEVEL = 1;
    int MAX_ZOOM_LEVEL = 10;

    if (panel->config == ZOOMPANEL_CONF_PROPORTIONAL) {
        MIN_ZOOM_LEVEL = -5;
        MAX_ZOOM_LEVEL = 25;
    }

    V2i mouse = GetMousePositioni();

    if (!CheckCollisionPointReci(mouse, draw_area)) { return; }
    if (mice_wheel() == 0) { return; }
    if (mice_wheel() > 0) {
        ++panel->zoom_level;
    } else {
        --panel->zoom_level;
    }
    panel->zoom_level = int_clamp(MIN_ZOOM_LEVEL, MAX_ZOOM_LEVEL, panel->zoom_level);

    float prev_zoom = panel->zoom;

    if (panel->config == ZOOMPANEL_CONF_PROPORTIONAL) {
        panel->zoom = ((float)panel->zoom_level + 10.f) / 10.f; // ???
        panel->zoom *= panel->zoom;
    } else if (panel->config == ZOOMPANEL_CONF_PIXEL_PERFECT) {
        panel->zoom = (float)panel->zoom_level;
        if (panel->zoom <= 0) { panel->zoom = 1; }
    }

    // Adjust pan

    V2f prev_size = v2f_mul(v2i_2f(panel->original_size), v2ff(prev_zoom));
    V2f new_size = v2f_mul(v2i_2f(panel->original_size), v2ff(panel->zoom));

    V2f prev_texture_topleft_origin = v2f_add(v2i_2f(draw_area.pos), v2i_2f(panel->offset_from_origin));
    V2f prev_pos = v2f_sub(v2i_2f(mouse), prev_texture_topleft_origin);

    V2f new_pos = v2f_mul(v2f_div(prev_pos, prev_size), new_size);
    V2f delta = v2f_sub(prev_pos, new_pos);

    panel->offset_from_origin = v2i_add(panel->offset_from_origin, v2f_2i(delta));
}


void zoompanel_reset_zoom_and_pan(Zoompanel *panel) {
    panel->zoom = 1;
    panel->zoom_level = 1;
    panel->offset_from_origin = (V2i) {0};
}


void zoompanel_process(Zoompanel *panel, V2i texture_size, Rect2i draw_area) {
    panel->original_size = texture_size;
    zoompanel__panning(panel, draw_area);
    zoompanel__zooming(panel, draw_area);
}

#endif // !ZOOMPANEL_H
