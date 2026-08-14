#ifndef RAYLIB_LITE_H
#define RAYLIB_LITE_H

#include "la_extra.h"
#include "portable_utils.h"

#if !defined(RAYLIB_LITE_COLOR_TRANSFORM)
    #define RAYLIB_LITE_COLOR_TRANSFORM(r,g,b,a) r,g,b,a
#endif

#include <stdint.h>
#if defined(__cplusplus)
    #define CLITERAL(type)      type
#else
    #define CLITERAL(type)      (type)
#endif

#define LIGHTGRAY  CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 200, 200, 200, 255 )}}   // Light Gray
#define GRAY       CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 130, 130, 130, 255 )}}   // Gray
#define DARKGRAY   CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 80, 80, 80, 255 )}}      // Dark Gray
#define YELLOW     CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 253, 249, 0, 255 )}}     // Yellow
#define GOLD       CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 255, 203, 0, 255 )}}     // Gold
#define ORANGE     CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 255, 161, 0, 255 )}}     // Orange
#define PINK       CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 255, 109, 194, 255 )}}   // Pink
#define RED        CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 230, 41, 55, 255 )}}     // Red
#define MAROON     CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 190, 33, 55, 255 )}}     // Maroon
#define GREEN      CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 0, 228, 48, 255 )}}      // Green
#define LIME       CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 0, 158, 47, 255 )}}      // Lime
#define DARKGREEN  CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 0, 117, 44, 255 )}}      // Dark Green
#define SKYBLUE    CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 102, 191, 255, 255 )}}   // Sky Blue
#define BLUE       CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 0, 121, 241, 255 )}}     // Blue
#define DARKBLUE   CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 0, 82, 172, 255 )}}      // Dark Blue
#define PURPLE     CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 200, 122, 255, 255 )}}   // Purple
#define VIOLET     CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 135, 60, 190, 255 )}}    // Violet
#define DARKPURPLE CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 112, 31, 126, 255 )}}    // Dark Purple
#define BEIGE      CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 211, 176, 131, 255 )}}   // Beige
#define BROWN      CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 127, 106, 79, 255 )}}    // Brown
#define DARKBROWN  CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 76, 63, 47, 255 )}}      // Dark Brown
#define WHITE      CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 255, 255, 255, 255 )}}   // White
#define BLACK      CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 0, 0, 0, 255 )}}         // Black
#define BLANK      CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 0, 0, 0, 0 )}}           // Blank (Transparent)
#define MAGENTA    CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 255, 0, 255, 255 )}}     // Magenta
#define RAYWHITE   CLITERAL(Color){{RAYLIB_LITE_COLOR_TRANSFORM( 245, 245, 245, 255 )}}   // My own White (raylib logo)

// R8G8B8A8 (32bit)
typedef union {
    struct {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    } rgba;
    struct {
        unsigned char b;
        unsigned char g;
        unsigned char r;
        unsigned char a;
    } bgra;
    uint32_t val;
    unsigned char c[4];
} Color;
wstatic_assert(sizeof(Color) == sizeof(u32));


void rgba_to_bgra(u32 *buffer, Rect2i rect, int stride) {
    const int x_max = rect.x + rect.width;
    const int y_max = rect.y + rect.height;
    for (int x = rect.x; x < x_max; ++x) {
        for (int y = rect.y; y < y_max; ++y) {
            Color *pixel = &((Color*)buffer)[y * stride + x];
            unsigned char bk = pixel->c[0];
            pixel->c[0] = pixel->c[2];
            pixel->c[2] = bk;
        }
    }
}

#endif
