/*
   DESCRITION:
   Very scuffed input system on top my other abstraction. Hopefully I eventually
   merge it with the other so we can have only one.
*/

#ifndef UI_MOUSE_INPUT_H
#define UI_MOUSE_INPUT_H

#include "winput.h"
#include "la_extra.h"
#include "portable_utils.h"
#include "execinfo.h"

#define UI_MOUSE_INPUT__DOUBLE_CLICK_MS 500
bool ui_mouse_available = true;
WinputFrame ui_winput_frame = { 0 };
long ui_mouse_input__last_left_click_timestamp = 0;
bool ui_mouse_input__double_click;

void print_call_stack(void) {
    void *buffer[20];
    int size = backtrace(buffer, 20);
    char **symbols = backtrace_symbols(buffer, size);
    if (symbols != NULL) {
        for (int i = 2; i < size; i++) {
            printf(ANSI_GRE"%s\n", symbols[i]);
        }
        free(symbols);
    }
}


void mice_consume(WinputMice btn) {
    ui_winput_frame.button[btn].pressed = false;
    ui_mouse_available = ui_winput_frame.button[btn].released;
    /* ↑↑↑ There is a case when both pressed and released are true on the same
       frame, this happens when the click is fast. This means
       we need to manually make it available again, because the normal way
       is through the released event but the logic code could run after
       that.
    */
}

bool mice_in_rect(Rect2i area) {
    if (!ui_mouse_available) return false;
    return CheckCollisionPointReci(winput_mouse_pos(), area);
}


bool mice_pressed_inside_and_consume(WinputMice btn, Rect2i area) {
    if (winput_frame_mice_pressed(&ui_winput_frame, btn) && CheckCollisionPointReci(winput_mouse_pos(), area)) {
        mice_consume(btn);
        return true;
    }
    return false;
}


bool mice_pressed(WinputMice btn) {
    return winput_frame_mice_pressed(&ui_winput_frame, btn);
}


bool mice_double_click(void) {
    bool v = ui_mouse_input__double_click;
    ui_mouse_input__double_click = false;
    if (v) { ui_mouse_input__last_left_click_timestamp = 0; }
    return v;
}

bool mice_pressed_consume(WinputMice btn) {
    if (winput_frame_mice_pressed(&ui_winput_frame, btn)) {
        mice_consume(btn);
        return true;
    }
    return false;
}


/// @Note: Release events can never be consumed/disabled/skipped.
bool mice_released(WinputMice btn) {
    return winput_frame_mice_released(&ui_winput_frame, btn);
}

bool mice_held(WinputMice btn) {
    if (!ui_mouse_available) { return false; }
    return winput_mice_held(btn);
}

int mice_wheel(void) {
    if (!ui_mouse_available) { return 0; }
    return (int)winput_frame_wheel(&ui_winput_frame);
}

void uimouseinput__frame_end(void) {
    if (ui_mouse_input__double_click) {
        ui_mouse_input__last_left_click_timestamp = 0;
    }
    ui_mouse_input__double_click = false;
}

void uimouseinput_glfw_mouse_button_callback(GLFWwindow* w, int button, int action, int mods) {
    (void)w; (void)mods;
    if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) {
        ui_mouse_available = true;
    }
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        long curr_time = get_system_ms();
        ui_mouse_input__double_click =
            (curr_time - ui_mouse_input__last_left_click_timestamp < UI_MOUSE_INPUT__DOUBLE_CLICK_MS);
        ui_mouse_input__last_left_click_timestamp = get_system_ms();
    }
}

#endif
