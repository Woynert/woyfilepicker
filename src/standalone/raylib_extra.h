#ifndef RAYLIB_EXTRA
#define RAYLIB_EXTRA

#include "arena_extra.h"
#include "portable_utils.h"
#include "raylib.h"
#include "raymath.h"
#include "strview.h"
#include "stdio.h"
#include "strbuf_extra.h"
#include "la.h"
#include "arena.h"
#include <sys/stat.h>
#include <sys/types.h>


typedef union Rect2 {
    struct {
        float x;
        float y;
        float width;
        float height;
    };
    struct {
        V2f pos;
        V2f size;
    };
    Rectangle rect;
} Rect2;

typedef union Rect2i {
    struct {
        int x;
        int y;
        int width;
        int height;
    };
    struct {
        V2i pos;
        V2i size;
    };
} Rect2i;

#define Rect2i_Fmt "Rect2i(%d, %d, %d, %d)"
#define Rect2i_Arg(r) r.x, r.y, r.width, r.height

Rect2 Rect2i_to_Rect2(Rect2i r)
{ return (Rect2) {{ (float)r.x, (float)r.y, (float)r.width, (float)r.height }}; }

Rect2i Rect2_to_Rect2i(Rect2 r)
{ return (Rect2i) {{ (int)r.x, (int)r.y, (int)r.width, (int)r.height }}; }

Vector2 v2i_to_Vector2(V2i v) { return (Vector2) { (float)v.x, (float)v.y }; }

V2i Vector2_to_v2i(Vector2 v) { return (V2i) {{ (int)v.x, (int)v.y }}; }

/* For drawing vertically inverted textures (i.e. BeginTextureMode) */
void DrawTextureRec_flipped (Texture2D texture, Rect2i source, V2i position, Color tint) {
    DrawTextureRec(
        texture,
        (Rectangle){
            (float)source.x,
            (float)texture.height - (float)source.height - (float)source.y,
            (float)source.width,
            -(float)source.height
        },
        v2i_to_Vector2(position), tint
    );
}

void DrawTexture_flipped(Texture2D texture, int posX, int posY, Color tint)
{
    DrawTextureRec(texture, (Rectangle) {
            (float)posX, (float)posY,
            (float)texture.width, (float)-texture.height }, (Vector2) { 0, 0 }, tint);
}f

/*
 * Extracted from rtext.c
 * Get index position for a unicode character on font, fallbacks to '�'
 */
int GetGlyphIndex_woy(Font font, int codepoint) {
    /* Defaults to '�' (0xFFFD). */

    int index = 0;
    if (!IsFontValid(font)) return index;
    int fallbackIndex = 0;
    for (int i = 0; i < font.glyphCount; i++)
    {
        if (font.glyphs[i].value == 0xFFFD) fallbackIndex = i;

        if (font.glyphs[i].value == codepoint)
        {
            index = i;
            break;
        }
    }
    if ((index == 0) && (font.glyphs[0].value != codepoint)) index = fallbackIndex;
    return index;
}

/* @returns byte cursor. */
int utf8_visually_nearest(const char *str, int size, int visual_char_count_target) {
    int count = 0;
    int codepoint_size = 0;
    for (int i = 0; i < size;) {
        if (count >= visual_char_count_target) {
            return i;
        }
        GetCodepointNext_woy(&str[i], &codepoint_size, size-i);
        i += codepoint_size;
        ++count;
    }
    return size;
}

/*
 * Extracted from rtext.c
 */
void DrawTextCodepoint_woy(Font font, int codepoint, Vector2 position, float fontSize, Color tint) {
    int index = GetGlyphIndex_woy(font, codepoint);
    float scaleFactor = fontSize/(float)font.baseSize;
    Rectangle dstRec = {
        position.x + (float)font.glyphs[index].offsetX*scaleFactor - (float)font.glyphPadding*scaleFactor,
        position.y + (float)font.glyphs[index].offsetY*scaleFactor - (float)font.glyphPadding*scaleFactor,
        (font.recs[index].width + 2.0f*(float)font.glyphPadding)*scaleFactor,
        (font.recs[index].height + 2.0f*(float)font.glyphPadding)*scaleFactor };
    Rectangle srcRec = {
        font.recs[index].x - (float)font.glyphPadding,
        font.recs[index].y - (float)font.glyphPadding,
        font.recs[index].width + 2.0f*(float)font.glyphPadding,
        font.recs[index].height + 2.0f*(float)font.glyphPadding };
    DrawTexturePro(font.texture, srcRec, dstRec, (Vector2){ 0, 0 }, 0.0f, tint);
}

/*
 * Extracted from rtext.c
 * @note: chars spacing is NOT proportional to fontSize.
 */
void DrawTextEx_strview(
    Font font, const strview_t string, Vector2 position, float fontSize,
    float spacing, float textLineSpacing, Color tint
) {
    float textOffsetY = 0;
    float textOffsetX = 0.0f;
    float scaleFactor = fontSize/(float)font.baseSize; 

    for (int i = 0; i < string.size;)
    {
        int codepointByteCount = 0;
        int codepoint = GetCodepointNext_woy(
                &string.data[i], &codepointByteCount, string.size-i);

        if (codepoint == '\n') {
            textOffsetY += (fontSize + textLineSpacing);
            textOffsetX = 0.0f;
        } else {
            if ((codepoint != ' ') && (codepoint != '\t')) {
                DrawTextCodepoint_woy(
                    font, codepoint,
                    (Vector2){
                        position.x + textOffsetX,
                        position.y + textOffsetY
                    },
                    fontSize, tint);
            }

            int index = GetGlyphIndex_woy(font, codepoint);
            if (font.glyphs[index].advanceX == 0) {
                textOffsetX += ((float)font.recs[index].width*scaleFactor + spacing);
            } else {
                textOffsetX += ((float)font.glyphs[index].advanceX*scaleFactor + spacing);
            }
        }
        i += codepointByteCount;   // Move text bytes counter to next codepoint
    }
}


void DrawTextEx_strview_i(
    Font font, const strview_t string, V2i position, int fontSize,
    int spacing, int textLineSpacing, Color tint
) {
    DrawTextEx_strview(font, string, v2i_to_Vector2(position), (float)fontSize, (float)spacing, (float)textLineSpacing, tint);
}


/*
 * Extracted from rtext.c
 * @note: Uses strview_t instead of Cstrings.
 */
Vector2 MeasureTextEx_woy(Font font, strview_t string, float fontSize, float spacing, float textLineSpacing)
{
    Vector2 textSize = { 0 };

    if ((font.texture.id == 0) || (string.data == NULL) || (string.size == 0) || (string.data[0] == '\0')) return textSize; // Security check

    int tempByteCounter = 0;        // Used to count longer text line num chars
    int byteCounter = 0;

    float textWidth = 0.0f;
    float tempTextWidth = 0.0f;     // Used to count longer text line width

    float textHeight = fontSize;
    float scaleFactor = fontSize/(float)font.baseSize;

    int letter = 0;                 // Current character
    int index = 0;                  // Index position in sprite font

    for (int i = 0; i < string.size;)
    {
        byteCounter++;

        int codepointByteCount = 0;
        letter = GetCodepointNext_woy(&string.data[i], &codepointByteCount, string.size -i);
        index = GetGlyphIndex_woy(font, letter);

        i += codepointByteCount;

        if (letter != '\n')
        {
            if (font.glyphs[index].advanceX > 0) textWidth += (float)font.glyphs[index].advanceX;
            else textWidth += (font.recs[index].width + (float)font.glyphs[index].offsetX);
        }
        else
        {
            if (tempTextWidth < textWidth) tempTextWidth = textWidth;
            byteCounter = 0;
            textWidth = 0;
            textHeight += (fontSize + textLineSpacing);
        }

        if (tempByteCounter < byteCounter) tempByteCounter = byteCounter;
    }

    if (tempTextWidth < textWidth) tempTextWidth = textWidth;

    textSize.x = tempTextWidth*scaleFactor + ((float)(tempByteCounter - 1)*spacing);
    textSize.y = textHeight;

    return textSize;
}

V2i MeasureTextEx_woyi(Font font, strview_t string, int fontSize, int spacing, int textLineSpacing) {
    return Vector2_to_v2i(MeasureTextEx_woy(font, string, (float)fontSize, (float)spacing, (float)textLineSpacing));
}


void DrawRectangleReci(Rect2i rect, Color color) {
    DrawRectanglePro(
        (Rectangle){ (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height },
        (Vector2){ 0.0f, 0.0f }, 0.0f, color);
}

void DrawRectangleLinesi(Rect2i rect, Color color, float thick) {
    DrawRectangleLinesEx(
        (Rectangle){ (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height },
        thick, color);
}

void DrawRectangleRecf(Rect2 rect, Color color) {
    DrawRectanglePro(
        (Rectangle){ rect.x, rect.y, rect.width, rect.height },
        (Vector2){ 0.0f, 0.0f }, 0.0f, color);
}

V2i GetMousePositioni(void) {
    Vector2 mouse = GetMousePosition();
    return (V2i) {{ (int)mouse.x, (int)mouse.y }};
}

void DrawTextureVi(Texture2D texture, V2i pos) {
    DrawTexture(texture, pos.x, pos.y, WHITE);
}

void DrawTextureScaled(Texture2D texture, Rect2i dest) {
    DrawTexturePro(
        texture,
        (Rectangle) {
            0, 0,
            (float)texture.width,
            (float)texture.height
        },
        (Rect2i_to_Rect2(dest)).rect,
        Vector2Zero(), 0, WHITE
    );
}

void DrawTextureScaled2(Texture2D texture, Rect2i dest, Rect2i source) {
    DrawTexturePro(
        texture,
        (Rect2i_to_Rect2(source)).rect,
        (Rect2i_to_Rect2(dest)).rect,
        Vector2Zero(), 0, WHITE
    );
}

void DrawTextureScaled2_flipped(Texture2D texture, Rect2i dest, V2i source) {
    DrawTexturePro(
        texture,
        (Rectangle) {
            0,
            (float)(texture.height - source.y),
            (float)source.x,
            (float)(-source.y)
        },
        (Rect2i_to_Rect2(dest)).rect,
        Vector2Zero(), 0, WHITE
    );
}

/*
   * AVOID USING THIS. This works on my machine but supposedly this should
   break on other hardware.
   * Under OpenGL there is no concept of integer coordinates, so to get closest
   to pixel perfect you need to add 0.5.
   * See https://stackoverflow.com/questions/10040961
   And https://github.com/raysan5/raylib/issues/2744
*/
void DrawLineFixed(V2i from, V2i to, float thick, Color tint) {
    DrawLineEx(
        Vector2Add(v2i_to_Vector2(from), (Vector2){0.5,0.5}),
        Vector2Add(v2i_to_Vector2(to), (Vector2){0.5,0.5}),
        thick, tint);
}


Rect2i Rect2i_from_two_positions(V2i a, V2i b) {
    int min_x = int_min(a.x, b.x);
    int min_y = int_min(a.y, b.y);
    int max_x = int_max(a.x, b.x);
    int max_y = int_max(a.y, b.y);
    return (Rect2i) {
        .x = min_x,
        .y = min_y,
        .width = max_x - min_x,
        .height = max_y - min_y
    };
}

/*
   @param Corner. Will be a corner.
   @param Point. Will be point contained within the square.
   */
Rect2i Rect2i_make_square_from_a_corner_and_a_point(V2i corner, V2i point) {
    Rect2i r;
    r.width = corner.x > point.x ? corner.x - point.x : point.x - corner.x;
    r.height = corner.y > point.y ? corner.y - point.y : point.y - corner.y;
    if (r.width > r.height) { r.height = r.width; }
    else { r.width = r.height; }
    r.x = corner.x - (point.x < corner.x ? r.width : 0);
    r.y = corner.y - (point.y < corner.y ? r.height : 0);
    return r;
}

bool Rect2i_collides_Rect2i (Rect2i lhs, Rect2i rhs) {
    // (left -X, right +X, up -Y, down +Y)
    int left1   = lhs.pos.x;
    int right1  = lhs.pos.x + lhs.size.x;
    int top1    = lhs.pos.y;
    int bottom1 = lhs.pos.y + lhs.size.y;
    int left2   = rhs.pos.x;
    int right2  = rhs.pos.x + rhs.size.x;
    int top2    = rhs.pos.y;
    int bottom2 = rhs.pos.y + rhs.size.y;
    return ! (
        left1 > right2 || left2 > right1 ||
        top1 > bottom2 || top2 > bottom1
    );
}

bool Rect2i_is_out_of_bounds(Rect2i rect, Rect2i bounds) {
    return (rect.x < bounds.x || rect.x >= bounds.width ||
        rect.y < bounds.x || rect.y >= bounds.height ||
        rect.x + rect.width > bounds.width ||
        rect.y + rect.height > bounds.height
    );
}

V2i Rect_fit_in_Rect_and_preserve_aspect_ratio(V2i container, V2i rect) {
    float scale_x = (float)container.x / (float)rect.x;
    float scale_y = (float)container.y / (float)rect.y;
    float scale_factor = fminf(scale_x, scale_y);
    return (V2i) {{
        .x = (int)((float)rect.x * scale_factor),
        .y = (int)((float)rect.y * scale_factor),
    }};
}

void DrawCheckerboard(Rect2i rect, Color color, int square_length) {
    int h_count = (int)ceilf(((float)rect.width / (float)square_length) / 2.f);
    int v_count = (int)ceilf(((float)rect.height / (float)square_length));
    int h_offset = 0, h_left, v_left = rect.height;
    for (int j = 0; j < v_count; ++j) {
        h_offset = !h_offset;
        h_left = rect.width - h_offset *square_length;
        for (int i = 0; i < h_count; ++i) {
            DrawRectangleReci((Rect2i){{
                rect.x + i * square_length *2 + h_offset * square_length,
                rect.y + j * square_length,
                int_min(square_length, h_left),
                int_min(square_length, v_left),
                }}, color
            );
            h_left -= square_length *2;
        }
        v_left -= square_length;
    }
}

// Finds a multiple of a number which makes it less or equal to a cap.
// @returns multiple.
// @retval 0. Not found.
int find_multiple_max_fit(int n, int cap) {
    if (n <= 0 || cap <= 0) return 0;
    return (int)floorf((float)cap / (float)n);
}


/* @Note: Use Rect2i_split_horizontally macro instead. */
void Rect2i__split_horizontally(const Rect2i area, int chunk_count, Rect2i *out_chunks, float *fractions) {
    Rect2i chunk = area;
    chunk.width = 0;
    for (int i = 0; i < chunk_count; ++i) {
        if (i == chunk_count -1) {
            // On last iteration ensure we cover all original width.
            chunk.width = area.x + area.width - chunk.x;
        } else {
            chunk.width = (int)(fractions[i] * (float)area.width);
        }
        out_chunks[i] = chunk;
        chunk.x += chunk.width;
    }
}

Rect2i Rect2i_stay_within_Rect2i(Rect2i r, Rect2i cont) {
    // There's probably a better way of doing this, but...
    if (r.y < cont.y) { r.y += cont.y - r.y; }
    if (r.x < cont.x) { r.x += cont.x - r.x; }
    if (r.x + r.width > cont.x + cont.width) { r.x += (cont.x + cont.width) - (r.x + r.width); }
    if (r.y + r.height > cont.y + cont.height) { r.y += (cont.y + cont.height) - (r.y + r.height); }
    return r;
}

Rect2i Rect2i_stay_centered_and_contained(Rect2i r, Rect2i cont) {
    if (r.width > cont.width) { r.width = cont.width; }
    if (r.height > cont.height) { r.height = cont.height; }
    r.x = (cont.width - r.width)/2;
    r.y = (cont.height - r.height)/2;
    return r;
}

/*
   // Usage example:
   Rect2i chunks[3];
   Rect2i_split_horizontally(line_box, 3, chunks, { 0.1f, 0.2f });
*/
#define Rect2i_split_horizontally(area, chunk_count, out_chunks, ...)            \
    do {                                                                         \
        float __fractions[countof(chunks) -1] = __VA_ARGS__;                     \
        wstatic_assert(countof(out_chunks) == (size_t)chunk_count);              \
        Rect2i__split_horizontally(area, chunk_count, out_chunks, __fractions);  \
    } while(0);

/*
typedef struct Rect2i_pair { Rect2i left; Rect2i right; } Rect2i_pair;

Rect2i Rect2i_slice_horizontally_get_left(Rect2i src, float offset, float size) {
    return (Rect2i) {
        .x = src.x + (int)((float)src.width * offset),
        .y = src.y,
        .width = (int)((float)src.width * size),
        .height = src.height
    };
}

Rect2i_pair Rect2i_split_vertically_px(Rect2i src, int offset_px) {
    return (Rect2i_pair) {
        .left = {
            .x = src.x,
            .y = src.y,
            .width = src.width,
            .height = offset_px
        },
        .right = {
            .x = src.x,
            .y = src.y + offset_px,
            .width = src.width,
            .height = src.height - offset_px
        },
    };
}

Rect2i_pair Rect2i_split_horizontally_px(Rect2i src, int offset_px) {
    return (Rect2i_pair) {
        .left = {
            .x = src.x,
            .y = src.y,
            .width = offset_px,
            .height = src.height
        },
        .right = {
            .x = src.x + offset_px,
            .y = src.y,
            .width = src.width - offset_px,
            .height = src.height
        },
    };
}
*/


/// @Note: Ranges are inclusive.
/// @Returns Font. Can fail, check with IsFontValid(...).
Font load_font_with_buncha_codepoints(const char* font_path, int font_size,
        int *ranges, int ranges_size
) {
    int total_codepoints = 0;
    for (int i = 0; i < ranges_size; i += 2) {
        total_codepoints += ranges[i+1] - ranges[i] +1; // Inclusive
    }

    int *codepoints = (int*)malloc((size_t)total_codepoints * sizeof(int));
    int codepoint_count = 0;

    for (int i = 0; i < ranges_size; i += 2) {
        for (int j = ranges[i]; j <= ranges[i+1]; ++j) {
            codepoints[codepoint_count] = j;
            ++codepoint_count;
        }
    }

    wassert(codepoint_count == total_codepoints);
    Font font = LoadFontEx(font_path, (int)font_size, codepoints, (int)codepoint_count);
    free(codepoints);
    return font;
}

#endif // !RAYLIB_EXTRA
