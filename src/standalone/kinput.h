/*
   After a quick look at raylib's rcore_desktop_glfw.c it seems
   GLFW key codes are compatible with raylib's.
   */

#ifndef KINPUT_H
#define KINPUT_H

#include "GLFW/glfw3.h"
#include "portable_utils.h"
#include "string.h"

// Global state

struct {
    struct {
        bool pressed;
        bool held;
        bool released;
    } key[GLFW_KEY_LAST+1];
} KINPUT = { 0 };

static inline bool kinput__key_valid(int key) { return int_in_range_inclusive(0, GLFW_KEY_LAST, key); }

void kinput_glfw_key_callback(GLFWwindow* w, int key, int scancode, int action, int mods) {
    (void)mods; (void)scancode; (void)w;
    if (!kinput__key_valid(key)) { return; }
    if (action == GLFW_PRESS) {
        KINPUT.key[key].pressed = true;
        KINPUT.key[key].held = true;
    } else if (action == GLFW_RELEASE) {
        KINPUT.key[key].released = true;
    }
}

// Call at frame end.
void kinput_frame_end(void) {
    for (int i = 0; i < GLFW_KEY_LAST; ++i) {
        if (KINPUT.key[i].released) {
            KINPUT.key[i].held = false;
        }
        KINPUT.key[i].released = false;
        KINPUT.key[i].pressed = false;
    }
}

bool kinput_key_pressed(int key) {
    return kinput__key_valid(key) ? KINPUT.key[key].pressed : false;
}

bool kinput_key_held(int key) {
    return kinput__key_valid(key) ? KINPUT.key[key].held : false;
}

bool kinput_key_released(int key) {
    return kinput__key_valid(key) ? KINPUT.key[key].released : false;
}

bool kinput_key_held_SHIFT(void) {
    return KINPUT.key[GLFW_KEY_LEFT_SHIFT].held || KINPUT.key[GLFW_KEY_RIGHT_SHIFT].held;
}

bool kinput_key_held_CTRL(void) {
    return KINPUT.key[GLFW_KEY_LEFT_CONTROL].held || KINPUT.key[GLFW_KEY_RIGHT_CONTROL].held;
}

bool kinput_key_held_ALT(void) {
    return KINPUT.key[GLFW_KEY_LEFT_ALT].held || KINPUT.key[GLFW_KEY_RIGHT_ALT].held;
}

void kinput_consume_pressed(int key) {
    if (kinput__key_valid(key)) KINPUT.key[key].pressed = false;
}

void kinput_consume_held(int key) {
    if (kinput__key_valid(key)) KINPUT.key[key].held = false;
}

void kinput_consume_released(int key) {
    if (kinput__key_valid(key)) KINPUT.key[key].released = false;
}

void kinput_consume_key(int key) {
    if (!kinput__key_valid(key)) return;
    KINPUT.key[key].pressed = false;
    KINPUT.key[key].held = false;
    KINPUT.key[key].released = false;
}

#endif
