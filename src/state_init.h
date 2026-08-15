#ifndef STATE_INIT_H
#define STATE_INIT_H

#include "state.h"
#include "stbtt_extra.h"
#include "wod_drawer.h"

void ctx_init(Ctx *ctx) {
    *ctx = (Ctx){0};
    ctx->framearena_root = ArenaRoot_create(1024 * 1024);
    ctx->framearena = ArenaRoot_get_arena(ctx->framearena_root);
}

void ctx_load_assets(Ctx *ctx) {
    //ctx->icon1 = load_image(cstr_SL("assets/imgdemox64.png"));
    ctx->icon1 = load_image(cstr_SL("assets/imgdemo3.png"));
    wassert(!wod_error());

    wod_file_t file = load_file("./assets/Roboto-Regular.ttf");
    wassert(!wod_error());
    free_file(file);

   const V2i ranges[] = { // Ranges are inclusive
      {{ 0xFFFD,  0xFFFD }},  // (�) codepoint
      {{ 32,      127 }},     // Basic latin
      {{ 0x00A1,  0x00FF }},  // C1 Controls and Latin-1 Supplement
      {{ 0x0100,  0x017F }},  // Latin Extended-A
      {{ 0x0180,  0x024F }},  // Latin Extended-B
      {{ 0x1F300, 0x1F5FF }}, // Miscellaneous Symbols and Pictographs
      {{ 0x1F600, 0x1F64F }}, // Emoticons
   };

    wod_font_t font = load_font(ctx->framearena, cstr_SL("./assets/Roboto-Regular.ttf"), 16, (V2i*)ranges, countofi(ranges));
    wassert(!wod_error());
    stbtt_print_bitmap((unsigned char*)font.bitmap.data, font.bitmap.size.x, font.bitmap.size.y);
    ctx->font1 = font;
    //free_font(font);
}


void ctx_free(Ctx *ctx) {
    free_image(ctx->icon1);
    ArenaRoot_free(&ctx->framearena_root);
}

#endif // !STATE_INIT_H
