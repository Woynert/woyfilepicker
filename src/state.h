#ifndef STATE_H
#define STATE_H

#include "la_extra.h"
#include "silk_wrap.h"

typedef struct Ctx {
    SilkCtx silk_ctx;
    V2i window_size;
} Ctx;

#endif
