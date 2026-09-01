#ifndef WINPUT_H
#define WINPUT_H

#include "GLFW/glfw3.h"
//#include "raylib.h"
#include "la.h"
#include "stdbool.h"
#include "assert.h"

/*
    USAGE:

    Step 1: Hook GLFW events from your application:

        void glfw_mouse_callback(GLFWwindow* w, int button, int action, int mods) {
            winput_glfw_mouse_button_callback(w, button, action, mods);
        }

        void glfw_scroll_callback(GLFWwindow* w, double xoffset, double yoffset) {
            winput_glfw_scroll_callback(w, xoffset, yoffset);
        }

        void hook_glfw_callbacks(void) {
            glfwSetMouseButtonCallback(window, glfw_mouse_callback);
            glfwSetScrollCallback(window, glfw_scroll_callback);
        }

        main (...) {
            ...
            InitWindow(...);
            hook_glfw_callbacks(); <-- Call after InitWindow.
            ...
        }

    Step 2: Modify your main loop to include this functions:

        while (!WindowShouldClose()) {
            ... Drawing ...
            winput_consume_all(); <-- Call it just before EndDrawing.
            EndDrawing();
        }
 */

typedef enum WinputMice {
    MouseLeft,
    MouseRight,
    MouseMiddle,
    MouseNavBack,
    MouseNavForward,
    WINPUT_MOUSE_LAST
} WinputMice;


typedef struct WinputFrame {
    struct {
        bool pressed;
        bool held;
        bool released;
        bool ignore_next_release;
    } button[WINPUT_MOUSE_LAST];
    float wheel_x;
    float wheel_y;
    V2i mouse_pos;
} WinputFrame;


WinputFrame winput__state = { 0 };


void winput_glfw_cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
    winput__state.mouse_pos = (V2i){{ (int)xpos, (int)ypos }};
}

V2i winput_mouse_pos(void) {
    return winput__state.mouse_pos;
}


void winput_glfw_mouse_button_callback(GLFWwindow* w, int button, int action, int mods)
{
    (void)w; (void)mods;
    int local_button;
    switch (button) {
        default: return;
        case GLFW_MOUSE_BUTTON_LEFT  : local_button = MouseLeft; break;
        case GLFW_MOUSE_BUTTON_MIDDLE: local_button = MouseMiddle; break;
        case GLFW_MOUSE_BUTTON_RIGHT : local_button = MouseRight; break;
        case GLFW_MOUSE_BUTTON_4     : local_button = MouseNavBack; break;
        case GLFW_MOUSE_BUTTON_5     : local_button = MouseNavForward; break;
    }

    if (action == GLFW_PRESS) {
        winput__state.button[local_button].pressed = true;
        winput__state.button[local_button].held = true;
        if (winput__state.button[local_button].ignore_next_release) {
            winput__state.button[local_button].ignore_next_release = false;
        }
    } else if (action == GLFW_RELEASE) {
        if (winput__state.button[local_button].ignore_next_release) {
            winput__state.button[local_button].ignore_next_release = false;
        } else {
            winput__state.button[local_button].released = true;
        }
    }
}


// @Note. Call at frame start.
void winput_sync_frame(WinputFrame *frame) {
    frame->wheel_x = winput__state.wheel_x;
    frame->wheel_y = winput__state.wheel_y;
    for (int i = MouseLeft; i < WINPUT_MOUSE_LAST; ++i) {
        frame->button[i].pressed  = winput__state.button[i].pressed;
        if (frame->button[i].pressed && frame->button[i].ignore_next_release) {
            frame->button[i].ignore_next_release = false; // TODO: Add a test case for this.
        }
        if (!frame->button[i].ignore_next_release) {
            frame->button[i].held = winput__state.button[i].held;
        }
        frame->button[i].released = winput__state.button[i].released;
        if (frame->button[i].released && frame->button[i].ignore_next_release) {
            frame->button[i].ignore_next_release = false;
            frame->button[i].released = false;
        }
    }
}


void winput_glfw_scroll_callback(GLFWwindow* w, double xoffset, double yoffset)
{
    (void)w;
    winput__state.wheel_y = (float)yoffset;
    winput__state.wheel_x = (float)xoffset;
}




/* @Note. Call at frame end. */
void winput_consume_all(void) {
    for (int i = MouseLeft; i < WINPUT_MOUSE_LAST; ++i) {
        winput__state.button[i].pressed = false;
        if (winput__state.button[i].released) {
            /* Held is reset only when Released was triggered. */
            winput__state.button[i].held = false;
        }
        winput__state.button[i].released = false;
    }

    winput__state.wheel_x = 0;
    winput__state.wheel_y = 0;
}


void winput_consume(WinputFrame *frame, WinputMice button, bool trigger_release) {
    (void)trigger_release;
    if (frame == NULL) { frame = &winput__state; }
    assert(button < WINPUT_MOUSE_LAST);
    frame->button[button].pressed             = false;
    //frame->button[button].held                = false;
    //frame->button[button].ignore_next_release = !trigger_release;
    //frame->button[button].released            = trigger_release;
    //frame->button[button].ignore_next_release = true;
    //frame->button[button].released            = trigger_release;
}

bool winput_frame_mice_pressed(WinputFrame *frame, WinputMice button) {
    assert(button < WINPUT_MOUSE_LAST);
    return frame->button[button].pressed;
}

bool winput_frame_mice_held(WinputFrame *frame, WinputMice button) {
    assert(button < WINPUT_MOUSE_LAST);
    return frame->button[button].held;
}

bool winput_frame_mice_released(WinputFrame *frame, WinputMice button) {
    assert(button < WINPUT_MOUSE_LAST);
    return frame->button[button].released;
}

float winput_frame_wheel(WinputFrame *frame) {
    return frame->wheel_y;
}

bool winput_mice_pressed(WinputMice button) {
    return winput_frame_mice_pressed(&winput__state, button);
}

bool winput_mice_held(WinputMice button) {
    return winput_frame_mice_held(&winput__state, button);
}

bool winput_mice_released(WinputMice button) {
    return winput_frame_mice_released(&winput__state, button);
}

float winput_wheel(void) {
    return winput__state.wheel_y;
}

float winput_wheel_h(void) {
    return winput__state.wheel_x;
}


#endif // !WINPUT_H
