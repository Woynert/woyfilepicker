/*
   textbox.h FEATURES:
   * Single line editing.
   * Mouse selection.
   * Cursor movement.
   * Auto scrolling.
   */
#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "stdlib.h"
#include "string.h"
#include "portable_utils.h"
#include "strbuf_extra.h"
#include "wstrview.h"

unsigned char textbox__mask[5] = {
    0x80, // 10000000: 1 byte length mask.
    0xE0, // 11100000: 2 bytes length mask.
    0xF0, // 11110000: 3 bytes length mask.
    0xF8, // 11111000: 4 bytes length mask.
    0xC0, // 11000000: Continuation byte only mask.
};
unsigned char textbox__prefix[5] = {
    0x00, // 0xxxxxxx: 1 byte length.
    0xC0, // 110xxxxx: 2 bytes length.
    0xE0, // 1110xxxx: 3 bytes length.
    0xF0, // 11110xxx: 4 bytes length.
    0x80, // 10xxxxxx: Continuation byte only.
};

typedef struct {
    struct {
        int size;
        int capacity;
        char *buffer;
    };
    int cursor;
    int scroll;
    bool is_selecting;
    int selection_cursor;
} Textbox;

void textbox__debug_print(Textbox *t) {
    printfd("size %d cap %d cursor %d",
            t->size, t->capacity, t->cursor);
    printfd(PRIstrw"|"PRIstrw,
            PRIstrarg(((strview_t){.data=t->buffer,.size=t->cursor})),
            PRIstrarg(((strview_t){.data=t->buffer+t->cursor,.size=t->size - t->cursor})));
}

int textbox__grow(Textbox *t, int min_capacity) {
    if (t->capacity >= min_capacity) { return 0; }
    char *new_buffer = NULL;
    if (!t->buffer) {
        new_buffer = (char*)malloc((size_t)min_capacity);
    } else {
        new_buffer = (char*)realloc(t->buffer, (size_t)min_capacity);
    }
    if (!new_buffer) { return -1; }
    t->capacity = min_capacity;
    t->buffer = new_buffer;
    return 0;
}

void textbox__snap_cursor_to_next_codepoint_boundary(Textbox *t) {
    while(t->cursor < t->size) {
        if ((t->buffer[t->cursor] & textbox__mask[4]) != textbox__prefix[4]) {
            break;
        }
        ++t->cursor;
    }
}

void textbox__insert_codepoint(Textbox *t, char cp[4], int length) {
    if (length <= 0 || length > 4) { return; }
    if (t->size >= t->capacity) {
        int err = textbox__grow(t, t->capacity + 256);
        if (err != 0) { printferr("Couldn't grow. OOM?"); }
    }
    //t->cursor = int_clamp(0, t->size, t->cursor);
    textbox__snap_cursor_to_next_codepoint_boundary(t);
    memmove(t->buffer +t->cursor +length, t->buffer +t->cursor, (size_t)(t->size - t->cursor));
    for (int i = 0; i < length; ++i) {
        t->buffer[t->cursor + i] = cp[i];
    }
    t->cursor += length;
    t->size += length;
}

void textbox__delete_range(Textbox *t, int from, int to) {
    if (from >= to) { return; }
    if (!int_in_range_inclusive(0, t->size, from)) { return; }
    if (!int_in_range_inclusive(0, t->size, to)) { return; }
    memmove(t->buffer + from, t->buffer + to, (size_t)(t->size - to));
    t->size -= to - from;
}

void textbox_backspace(Textbox *t) {
    textbox__snap_cursor_to_next_codepoint_boundary(t);
    // Delete until previous codepoint boundary.
    // Scan back until you find something that is not 10xxxxxx.
    if (t->cursor <= 0) { return; }
    int start = t->cursor;
    --t->cursor;
    while (t->cursor > 0) {
        if ((t->buffer[t->cursor] & textbox__mask[4]) != textbox__prefix[4]) { break; }
        --t->cursor;
    }
    textbox__delete_range(t, t->cursor, start);
}

void textbox_cursor_left(Textbox *t) {
    if (t->cursor <= 0) { return; }
    --t->cursor;
    while (t->cursor > 0) {
        if ((t->buffer[t->cursor] & textbox__mask[4]) != textbox__prefix[4]) { break; }
        --t->cursor;
    }
}

void textbox_cursor_right(Textbox *t) {
    if (t->cursor >= t->size) { return; }
    ++t->cursor;
    textbox__snap_cursor_to_next_codepoint_boundary(t);
}

void textbox_delete(Textbox *t) {
    if (t->cursor >= t->size) { return; }
    textbox_cursor_right(t);
    textbox_backspace(t);
}

int textbox_set_buffer(Textbox *t, const char *buf, int size) {
    int err = textbox__grow(t, size);
    if (err != 0) { printferr("Couldn't grow. OOM?"); return 0; }
    if (size > 0) { memmove(t->buffer, buf, (size_t)size); }
    t->size = size;
    t->cursor = size;
    return 0;
}

void textbox_add_codepoint(Textbox *t, unsigned int codepoint) {
    // (See man 7 utf-8) Codepoint to utf8 table (ranges are inclusive):
    // U+0000 to U+007F    | 1 byte  | 0xxxxxxx
    // U+0080 to U+07FF    | 2 bytes | 110xxxxx 10xxxxxx
    // U+0800 to U+FFFF    | 3 bytes | 1110xxxx 10xxxxxx 10xxxxxx
    // U+10000 to U+10FFFF | 4 bytes | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    unsigned char bytes[4] = { 0 };
    int length = 0;
    if (codepoint <= 0x7F) {
        length = 1;
        bytes[0] = codepoint & 0xFF;
    } else if (codepoint <= 0x07FF) {
        length = 2;
        bytes[1] = ((codepoint & 0xFF) & (0xFF ^ textbox__mask[4])) | textbox__prefix[4]; codepoint >>= 6;
        bytes[0] = ((codepoint & 0xFF) & (0xFF ^ textbox__mask[1])) | textbox__prefix[1];
    } else if (codepoint <= 0xFFFF) {
        length = 3;
        bytes[2] = ((codepoint & 0xFF) & (0xFF ^ textbox__mask[4])) | textbox__prefix[4]; codepoint >>= 6;
        bytes[1] = ((codepoint & 0xFF) & (0xFF ^ textbox__mask[4])) | textbox__prefix[4]; codepoint >>= 6;
        bytes[0] = ((codepoint & 0xFF) & (0xFF ^ textbox__mask[2])) | textbox__prefix[2];
    } else if (codepoint <= 0x10FFFF) {
        length = 4;
        bytes[3] = ((codepoint & 0xFF) & (0xFF ^ textbox__mask[4])) | textbox__prefix[4]; codepoint >>= 6;
        bytes[2] = ((codepoint & 0xFF) & (0xFF ^ textbox__mask[4])) | textbox__prefix[4]; codepoint >>= 6;
        bytes[1] = ((codepoint & 0xFF) & (0xFF ^ textbox__mask[4])) | textbox__prefix[4]; codepoint >>= 6;
        bytes[0] = ((codepoint & 0xFF) & (0xFF ^ textbox__mask[3])) | textbox__prefix[3];
    }
    if (length == 0) { printferr("W: Invalid codepoint. (%d)", codepoint); return; }
    textbox__insert_codepoint(t, (char*)bytes, length);
}

void textbox_set_cursor(Textbox *t, int cursor) {
    t->cursor = int_clamp(0, t->size, cursor);
    textbox__snap_cursor_to_next_codepoint_boundary(t);
}

void textbox_free(Textbox *t) {
    if (t->buffer) { free(t->buffer); }
    *t = (Textbox) { 0 };
}

#endif
