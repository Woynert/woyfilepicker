#ifndef WOD_TEXT_H
#define WOD_TEXT_H

#include <stdio.h>
#include "portable_utils.h"
#include "stb_rect_pack.h"
#include "stb_truetype.h"
#include "arena.h"
#include "la.h"


/// @Returns error.
int stbtt_packed_bitmap_calculate_minimum_height_for_given_width(
      Arena scratch, int width, int *out_height, int padding,
      const unsigned char *fontdata, int font_index,
      stbtt_pack_range *ranges, int num_ranges
) {
   int total_codepoints = 0;
   for (int i=0; i < num_ranges; ++i) {
      total_codepoints += ranges[i].num_chars;
   }
   int num_nodes = width - padding;
   stbrp_rect *rects = arena_new(&scratch, stbrp_rect, total_codepoints);
   stbrp_node *nodes = arena_new(&scratch, stbrp_node, num_nodes);
   if (rects == NULL || nodes == NULL) { return -1; }

   stbtt_fontinfo info = { 0 };
   int err = stbtt_InitFont(&info, fontdata, stbtt_GetFontOffsetForIndex(fontdata, font_index));
   if (err == 0) { return -1; }

   {
      stbtt_pack_context spc = {
         .h_oversample = 1,
         .v_oversample = 1,
         .skip_missing = 0,
         .padding = padding,
      };
      stbtt_PackFontRangesGatherRects(&spc, &info, ranges, num_ranges, rects);
      // Note:
      // * Only purpose is to get the rects.
      // * Returns total_codepoints which we already have so it's safe to ignore.
      // * This function only requires:
      //   spc->h_oversample, spc->v_oversample, spc->skip_missing, spc->padding.
   }

   // Find optimal height by binary search.

   int lower_range = 0;
   int upper_range = 1024 * 1024 * 10; // Search range [0 - 10 MiB].
   int smallest_optimal_height = 0;
   int possible_height = upper_range / 2;
   for (int tries = 0; tries < 100; ++tries) {
      if (upper_range == lower_range) { break; }
      printfd("Trying height %d up %d down %d", possible_height, upper_range, lower_range);

      // flag all characters as NOT packed.
      for (int i = 0; i < num_ranges; ++i) {
         for (int j = 0; j < ranges[i].num_chars; ++j) {
            ranges[i].chardata_for_range[j].x0 =
            ranges[i].chardata_for_range[j].y0 =
            ranges[i].chardata_for_range[j].x1 =
            ranges[i].chardata_for_range[j].y1 = 0;
         }
      }

      // Try pack.
      stbrp_context pack_ctx = { 0 };
      stbrp_init_target(&pack_ctx, width - padding, possible_height - padding, nodes, num_nodes);
      bool all_rects_were_packed = 1 == stbrp_pack_rects(&pack_ctx, rects, total_codepoints);

      if (all_rects_were_packed) {
         printfd(ANSI_BLU"%d WILL work", possible_height);
         smallest_optimal_height = possible_height;
         upper_range = possible_height;
         possible_height -= (possible_height - lower_range) / 2;
      } else {
         printfd(ANSI_RED"%d wont work", possible_height);
         lower_range = possible_height +1;
         possible_height += (upper_range - possible_height) / 2;
      }
   }

   if (smallest_optimal_height != 0) {
      *out_height = smallest_optimal_height;
      return 0; // Success.
   }

   return -1;
}


stbtt_pack_range *make_stbtt_pack_range(int font_size, V2i *ranges, int ranges_size, Arena *perm)
{
   stbtt_pack_range *pack_ranges = arena_new(perm, stbtt_pack_range, ranges_size);
   for (int i = 0; i < ranges_size; ++i) {
      int start = ranges[i].c[0];
      int end   = ranges[i].c[1];
      stbtt_packedchar *chardata = arena_new(perm, stbtt_packedchar, end - start +1); // Inclusive.
      pack_ranges[i] = (stbtt_pack_range) {
         .font_size = (float)font_size,
         .first_unicode_codepoint_in_range = start,
         .num_chars = end - start +1,
         .chardata_for_range = chardata,
      };
   }
   return pack_ranges;
}

/// @Returns error.
int stbtt_create_bitmap_ranges(
      const unsigned char *font_data, int font_index, int padding,
      unsigned char *bitmap, int width, int height, int stride,
      stbtt_pack_range *ranges, int num_ranges
) {
   stbtt_pack_context pack_ctx = { 0 };
   int err = stbtt_PackBegin(&pack_ctx, bitmap, width, height, stride, padding, NULL);
   if (err == 0) { return -1; }
   // TODO: Upperscaling?
   err = stbtt_PackFontRanges(&pack_ctx, font_data, font_index, ranges, num_ranges);
   if (err == 0) { return -1; }
   stbtt_PackEnd(&pack_ctx);
   return 0;
}

void stbtt_print_bitmap(const unsigned char* bitmap, int width, int height) {
   for (int j=0; j < height; ++j) {
      for (int i=0; i < width; ++i) {
         putchar(" .:ioVM@"[bitmap[j * width + i]>>5]);
      }
      putchar('\n');
   }
}

//void test2(void) {
   //fread(file_buffer, 1, 1000000, fopen("./assets/Roboto-Regular.ttf", "rb"));

   //ArenaRoot arenaroot = ArenaRoot_create(1024 * 1024);
   //Arena arena = ArenaRoot_get_arena(arenaroot);

   //const int ranges[] = { // Ranges are inclusive
      //0xFFFD,  0xFFFD,  // (�) codepoint
      //32,      127,     // Basic latin
      //0x00A1,  0x00FF,  // C1 Controls and Latin-1 Supplement
      //0x0100,  0x017F,  // Latin Extended-A
      //0x0180,  0x024F,  // Latin Extended-B
      //0x1F300, 0x1F5FF, // Miscellaneous Symbols and Pictographs
      //0x1F600, 0x1F64F, // Emoticons
   //};
   //int ranges_size = countofi(ranges);

   //int font_size = 10;
   //int font_index = 0;
   //int padding = 1;
   //int width = 300;
   //int height = 0;

   //stbtt_pack_range *pack_ranges = make_stbtt_pack_range(font_size, (int*)ranges, ranges_size, &arena);
   
   //int err = stbtt_packed_bitmap_calculate_minimum_height_for_given_width(
      //arena, width, &height, padding,
      //(const unsigned char*)file_buffer, font_index,
      //pack_ranges, ranges_size/2 );
   //printfd("calculate: err %d got height %d", err, height);
   //if (err) { return; }

   //[>unsigned char* bitmap = arena_new(&arena, unsigned char, width*height);<]
   //unsigned char* bitmap = (unsigned char*)malloc(sizeof(unsigned char) * (size_t)(width*height));
   //err = stbtt_create_bitmap_ranges(file_buffer, font_index, padding, bitmap, width, height, 0, pack_ranges, ranges_size/2);
   //printfd("stbtt_create_bitmap_ranges: err %d");
   //if (err) { return; }

   //print_bitmap(bitmap, width, height);
   //free(bitmap);

   //ArenaRoot_free(&arenaroot);
//}

#endif
