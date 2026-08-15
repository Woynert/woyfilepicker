#ifndef STATE_H
#define STATE_H

#include "la_extra.h"
#include "silk_wrap.h"

typedef struct Ctx {
    V2i window_size;
    Image icon1;
    ArenaRoot framearena_root;
    Arena framearena;
} Ctx;

#endif
