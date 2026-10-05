#ifndef FOCUSER_EXTRA_H
#define FOCUSER_EXTRA_H

#include "kinput.h"
#include "state.h"
#include "focuser.h"

void focuser_frame_start(Ctx *ctx, Focuser *f) {
    // Here you should umhh...
    /*
       Not here but on the glfw callback make sure to relay those actions to your widget...
       Then uhh I guess you could consume those...
       But what about consuming those by polling all at once instead of doing it when they're coming?
       */
}

void focuser_process_frame_end(Ctx *ctx) {
    if (kinput_key_pressed(GLFW_KEY_TAB)) {
        focuser_advance_focus(&ctx->focuser, !kinput_key_held_SHIFT());
        focuser_print_debug(&ctx->focuser);
    }
    focuser__swap(&ctx->focuser);
}

#endif
