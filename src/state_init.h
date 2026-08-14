#ifndef STATE_INIT_H
#define STATE_INIT_H

#include "state.h"
#include "wod_drawer.h"

void ctx_init(Ctx *ctx) {
    
}

void ctx_load_assets(Ctx *ctx) {
    //ctx->icon1 = load_image(cstr_SL("assets/imgdemox64.png"));
    ctx->icon1 = load_image(cstr_SL("assets/imgdemox32.png"));
    wassert(wod_get_error() == 0);
}


void ctx_free(Ctx *ctx) {
    free_image(ctx->icon1);
}

#endif // !STATE_INIT_H
