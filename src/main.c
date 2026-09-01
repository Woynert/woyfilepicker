#include "operations.h"
#include "textbox.h"
#include "wod_drawer.h"
#include "state_init.h"
#define DBUF_IMG_T  Image
#define DBUF_FONT_T wod_font_t
#include "drawbuffer.h"
#include "kinput.h"
#include "olivec_wrap.h"
#include "portable_utils.h"
#include <X11/Xlib.h>
#include "x11back.h"
#include "la_extra.h"
#include "ui.h"
#include "file_utils.h"

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define LA_IMPLEMENTATION
#include "la.h"

static Textbox textbox = { 0 };
void textbox_debug_DELME(void) {
    
}

void glfw_mouse_callback(GLFWwindow* w, int button, int action, int mods) {
    winput_glfw_mouse_button_callback(w, button, action, mods);
    uimouseinput_glfw_mouse_button_callback(w, button, action, mods);
    MUST_REDRAW = true;
}
void glfw_scroll_callback(GLFWwindow* w, double xoffset, double yoffset) {
    winput_glfw_scroll_callback(w, xoffset, yoffset);
    MUST_REDRAW = true;
}
void glfw_cursor_pos_callback(GLFWwindow *w, double xpos, double ypos) {
    winput_glfw_cursor_pos_callback(w, xpos, ypos);
    MUST_REDRAW = true;
}
void glfw_key_callback(GLFWwindow* w, int key, int scancode, int action, int mods) {
    kinput_glfw_key_callback(w, key, scancode, action, mods);
    MUST_REDRAW = true;
    if (kinput_key_pressed(GLFW_KEY_Q)) {
        MUST_CLOSE = true;
    }
    textbox_glfw_key_callback(&textbox, key, scancode, action, mods);
}
void glfw_char_callback(GLFWwindow* w, unsigned int codepoint) {
    textbox_add_codepoint(&textbox, codepoint);
    if (kinput_key_pressed(GLFW_KEY_T)) {
        textbox_add_codepoint(&textbox, 0x0041);
        textbox_add_codepoint(&textbox, 0x007A);
        textbox_add_codepoint(&textbox, 0x00A9);
        textbox_add_codepoint(&textbox, 0x00FF);
        textbox_add_codepoint(&textbox, 0x0100);
        textbox_add_codepoint(&textbox, 0x07FF);
        textbox_add_codepoint(&textbox, 0x0800);
        textbox_add_codepoint(&textbox, 0xFFFF);
        textbox_add_codepoint(&textbox, 0x10000);
        textbox_add_codepoint(&textbox, 0x1F600);
    }
    textbox__debug_print(&textbox);
    MUST_REDRAW = true;
}
void glfw_window_size_callback (GLFWwindow *w, int width, int height) {
    Ctx *ctx = (Ctx*)glfwGetWindowUserPointer(w);
    ctx->window_size = (V2i) {{ width, height }};
    MUST_RESIZE = true;
    MUST_REDRAW = true;
}
void glfw_window_refresh_callback (GLFWwindow *w) {
    Ctx *ctx = (Ctx*)glfwGetWindowUserPointer(w);
    MUST_REDRAW = true;
}
void hook_glfw_callbacks(GLFWwindow* w, Ctx *ctx) {
    glfwSetWindowUserPointer(w, ctx);
    glfwSetCursorPosCallback(w, glfw_cursor_pos_callback);
    glfwSetMouseButtonCallback(w, glfw_mouse_callback);
    glfwSetScrollCallback(w, glfw_scroll_callback);
    glfwSetKeyCallback(w, glfw_key_callback);
    glfwSetCharCallback(w, glfw_char_callback);
    glfwSetWindowSizeCallback(w, glfw_window_size_callback);
    glfwSetWindowRefreshCallback(w, glfw_window_refresh_callback);
}

void setup(void) {
}


int main(void) {
    GLFWwindow* window;
    V2i initial_win_size = {{ 640, 480 }};
    if (!glfwInit()) { printfd("ERR: Failed to glfwInit."); return -1; }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(initial_win_size.x, initial_win_size.y, "File Explorer", NULL, NULL);
    if (!window) {
        printfd("ERR: Failed to create window.");
        glfwTerminate(); return -1;
    }
    glfwMakeContextCurrent(window);

    /// ↓↓↓

    Ctx __ctx = { 0 };
    Ctx *ctx = &__ctx;
    hook_glfw_callbacks(window, ctx);
    ctx_init(ctx);
    ctx_setup(ctx);
    update_bookmarks(ctx);
    refresh_listing(ctx);
    debug_print_listing(ctx);
    ctx->window_size = initial_win_size;
    wod_set_drawer(olivewrap_make_drawer());
    x11back_init(window);
    dbuf_init();
    dbuf_setup_callbacks(
        &draw_rect,
        &draw_frame,
        &draw_text,
        &draw_image_ext,
        &draw_scissor
    );
    MUST_RESIZE = true; // <-- Trigger buffers to resize.

    wassert(is_path_dir(cstr("/tmp/"), ctx->framearena));

    long prev_draw_timestamp = get_system_ms();
    int ticks = 0;
    int fps_calculation = 0;
    long long TARGET_FRAME_NS = (long long)(sec2ns(1.f/60.f));
    long long FRAME_START_TIME_NS;
    long long last_frame_timestamp_ns = 0;

    glfwPollEvents();
    while (!glfwWindowShouldClose(window) && !MUST_CLOSE)
    {
        glfwPollEvents();
        {
            winput_sync_frame(&ui_winput_frame); // TODO: Move me.
        }
        FORCE_UI_REDRAW = false;
        FRAME_START_TIME_NS = get_system_ns();

        ++ticks;
        long curr_time_ms = get_system_ms();
        if (curr_time_ms - prev_draw_timestamp > 1000) {
            prev_draw_timestamp = curr_time_ms;
            fps_calculation = ticks;
            ticks = 0;
        }
        /*printf("FPS %d\n", fps_calculation);*/

        if (MUST_RESIZE) {
            MUST_RESIZE = false;
            FORCE_UI_REDRAW = true;
            ctx->window_size = (V2i) {{ int_max(2, ctx->window_size.x), int_max(2, ctx->window_size.y) }};
            int err = x11_ensure_size(ctx->window_size);
            if (err == 0) {
                char *buffer = x11_get_buffer();
                memset(buffer, 0, (size_t)(ctx->window_size.x * ctx->window_size.y * 4));
                buffer = x11_swap_buffer();
                wod_set_buffer((u32*)buffer, ctx->window_size, ctx->window_size.x);
            } else {
                printfd("ERR: x11 Couldn't ensure size.");
            }
        }

        if (MUST_REDRAW) {
            MUST_REDRAW ^= 1;

            {
                draw_all(ctx, FORCE_UI_REDRAW);
            }


            x11_draw_texture();
        }

        if (mice_pressed(MouseLeft)) {
            printfd(ANSI_RED"MOUSE LEFT PRESSED");
        }
        if (kinput_key_pressed(GLFW_KEY_SPACE)) {
            long start = get_system_ns();
            refresh_listing(ctx);
            long end = get_system_ns();
            debug_print_listing(ctx);
            printfd(ANSI_MAG"refresh_listing took %f", ((double)end-(double)start)/(1000000000.f));
        }
        if (kinput_key_pressed(GLFW_KEY_A)) {
            navigate_backwards(ctx);
        }
        if (kinput_key_pressed(GLFW_KEY_W)) {
            navigate_parent_dir(ctx);
        }
        if (kinput_key_pressed(GLFW_KEY_D)) {
            navigate_forward(ctx);
        }

        ctx->framearena = ArenaRoot_get_arena(ctx->framearena_root);

        kinput_frame_end();
        uimouseinput__frame_end();
        winput_consume_all();
        long long frame_time = (get_system_ns() - FRAME_START_TIME_NS);
        long long time_since_last = get_system_ns() - last_frame_timestamp_ns;
        /*printfd("Frame: Ideal %.4fms, Actual %.4fms, FPS %.2f",*/
                /*ns2msf((float)TARGET_FRAME_NS), ns2msf((float)(time_since_last)),*/
                /*1000.f / ns2msf((float)time_since_last)*/
            /*);*/
        last_frame_timestamp_ns = get_system_ns();
        sleep_ns(long_long_max(0, TARGET_FRAME_NS - frame_time));
    }

    ctx_free(ctx);
    dbuf_deinit();
    glfwTerminate();
    printfd("End");
    return 0;
}
