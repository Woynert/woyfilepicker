#include "kinput.h"
#include "olivec_wrap.h"
#include "portable_utils.h"
#include <X11/Xlib.h>
#include "state_init.h"
#include "x11back.h"
#include "la_extra.h"
#include "silk.h"
#include "ui.h"
#include "silk_wrap.h"
#include "wod_drawer.h"

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define LA_IMPLEMENTATION
#include "la.h"

bool must_close = false;
bool must_redraw = false;
bool must_resize = false;
bool force_ui_redraw = false;

void glfw_mouse_callback(GLFWwindow* w, int button, int action, int mods) {
    winput_glfw_mouse_button_callback(w, button, action, mods);
}
void glfw_scroll_callback(GLFWwindow* w, double xoffset, double yoffset) {
    winput_glfw_scroll_callback(w, xoffset, yoffset);
}
void glfw_cursor_pos_callback(GLFWwindow *w, double xpos, double ypos) {
    winput_glfw_cursor_pos_callback(w, xpos, ypos);
    must_redraw = true;
}
void glfw_key_callback(GLFWwindow* w, int key, int scancode, int action, int mods) {
    kinput_glfw_key_callback(w, key, scancode, action, mods);
    must_redraw = true;
    if (kinput_key_pressed(GLFW_KEY_Q)) {
        must_close = true;
    }
}
void glfw_window_size_callback (GLFWwindow *w, int width, int height) {
    Ctx *ctx = (Ctx*)glfwGetWindowUserPointer(w);
    ctx->window_size = (V2i) {{ width, height }};
    must_resize = true;
    must_redraw = true;
}
void glfw_window_refresh_callback (GLFWwindow *w) {
    Ctx *ctx = (Ctx*)glfwGetWindowUserPointer(w);
    must_redraw = true;
}
void hook_glfw_callbacks(GLFWwindow* w, Ctx *ctx) {
    glfwSetWindowUserPointer(w, ctx);
    glfwSetCursorPosCallback(w, glfw_cursor_pos_callback);
    glfwSetMouseButtonCallback(w, glfw_mouse_callback);
    glfwSetScrollCallback(w, glfw_scroll_callback);
    glfwSetKeyCallback(w, glfw_key_callback);
    glfwSetWindowSizeCallback(w, glfw_window_size_callback);
    glfwSetWindowRefreshCallback(w, glfw_window_refresh_callback);
}

void test1(void) {
    wod_file_t file = load_file("./assets/imgdemox32.png");
    wassert(!wod_error());
    free_file(file);
}

int main(void) {
    GLFWwindow* window;
    V2i initial_win_size = {{ 640, 480 }};
    if (!glfwInit()) { printfd("ERR: Failed to glfwInit."); return -1; }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(initial_win_size.x, initial_win_size.y, "Hello World", NULL, NULL);
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
    ctx_load_assets(ctx);
    ctx->window_size = initial_win_size;

    // INIT WOOD DRAWER
    wod_set_drawer(olivewrap_make_drawer());

    /*silk_ctx_curr = &ctx->silk_ctx;*/
    /*silk_ctx_init(silk_ctx_curr);*/
    must_resize = true; // <-- Trigger buffers to resize.

    x11back_init(window);
    drawbuf_init();
    drawbuf__DrawRectCallback = &draw_rect;


    long prev_draw_timestamp = get_system_ms();
    int ticks = 0;
    int fps_calculation = 0;
    long long TARGET_FRAME_NS = (long long)(sec2ns(1.f/60.f));
    long long FRAME_START_TIME_NS;
    long long last_frame_timestamp_ns = 0;

    glfwPollEvents();
    while (!glfwWindowShouldClose(window) && !must_close)
    {
        force_ui_redraw = false;
        FRAME_START_TIME_NS = get_system_ns();

        ++ticks;
        long curr_time_ms = get_system_ms();
        if (curr_time_ms - prev_draw_timestamp > 1000) {
            prev_draw_timestamp = curr_time_ms;
            fps_calculation = ticks;
            ticks = 0;
        }
        printf("FPS %d\n", fps_calculation);

        if (must_resize) {
            must_resize = false;
            force_ui_redraw = true;
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

        if (must_redraw) {
            must_redraw ^= 1;

            {
                draw_all(ctx, force_ui_redraw);
            }


            x11_draw_texture();
        }

        ctx->framearena = ArenaRoot_get_arena(ctx->framearena_root);

        glfwPollEvents();
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
    drawbuf_deinit();
    glfwTerminate();
    printfd("End");
    return 0;
}
