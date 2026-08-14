#include "kinput.h"
#include "portable_utils.h"
/*#include "x11_platform.h"*/
#include <X11/Xlib.h>
#include "state_init.h"
#include "x11back.h"
#include "la_extra.h"
#include "silk.h"
#include "ui.h"
#include "silk_wrap.h"
/*#include <X11/Xlib.h>*/

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define LA_IMPLEMENTATION
#include "la.h"

bool must_redraw = false;
bool must_resize = false;

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
}
void glfw_window_size_callback (GLFWwindow *w, int width, int height) {
    Ctx *ctx = (Ctx*)glfwGetWindowUserPointer(w);
    ctx->window_size = (V2i) {{ width, height }};
    must_resize = true;
    /*must_redraw = true;*/
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


int main(void) {
    GLFWwindow* window;
    V2i initial_win_size = {{ 640, 480 }};

    Ctx __ctx = { 0 };
    Ctx *ctx = &__ctx;

    if (!glfwInit()) { printfd("ERR: Failed to glfwInit."); return -1; }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(initial_win_size.x, initial_win_size.y, "Hello World", NULL, NULL);
    if (!window) {
        printfd("ERR: Failed to create window.");
        glfwTerminate(); return -1;
    }
    glfwMakeContextCurrent(window);

    hook_glfw_callbacks(window, ctx);
    ctx_init(ctx);
    ctx->window_size = initial_win_size;

    /*V2i silk_buffer_size = ;*/
    u8* silk_buffer = (u8*)malloc((size_t)(3000 * 3000) * 4);

    /*SilkCtx silk_ctx = { 0 };*/
    silk_ctx_curr = &ctx->silk_ctx;
    silk_ctx_init(silk_ctx_curr);
    must_resize = true; // <-- Trigger buffers to resize.
    /*silk_resize(silk_ctx_curr, ctx->window_size);*/
    /*silk_resize(silk_ctx_curr, (V2i) {{*/
        /*ctx->window_size.x),*/
        /*int_max(SILK_PIXELBUFFER_HEIGHT, ctx->window_size.y)*/
    /*}});*/

    x11back_init(window);
    /*x11back_init(window, (V2i){{ silk_buffer_size.x, silk_buffer_size.y }});*/
    drawbuf_init();
    drawbuf__DrawRectCallback = &silk_DrawRectCallback;


    long fps_calculation_last_timestamp = 0;
    long prev_draw_timestamp = get_system_ms();
    long max_draw_wait_ms = 17;
    /*const long mimo = max_draw_wait_ms;*/
    double GLFW_EVENT_TIMEOUT_SECS = 1;
    int ticks = 0;
    int fps_calculation = 0;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        /*sleep_ms(16);*/

        ++ticks;
        long curr_time_ms = get_system_ms();
        long curr_time_ms_2 = 0;
        if (curr_time_ms - prev_draw_timestamp > 1000) {
            prev_draw_timestamp = curr_time_ms;
            fps_calculation = ticks;
            ticks = 0;
        }
        printf("FPS %d\n", fps_calculation);


        /*glfwWaitEventsTimeout(1.0/60.0);*/
        /*if (!must_redraw) { continue; }*/
        /*curr_timestamp = get_system_ms();*/
        /*if ((curr_timestamp - prev_draw_timestamp) < max_draw_wait_ms) { continue; }*/

        /*if (!must_redraw) { continue; }*/
        if (must_resize) {
            must_resize ^= 1;
            ctx->window_size = (V2i) {{ int_max(2, ctx->window_size.x), int_max(2, ctx->window_size.y) }};

            int err = x11_ensure_size(ctx->window_size);
            /*int err = 0;*/
            if (err == 0) {
                silk_set_buffer(silk_ctx_curr, (char*)silk_buffer, ctx->window_size, ctx->window_size.x);
            } else {
                printfd("ERR: x11 Couldn't ensure size.");
            }
        }

        /*silkClearPixelBufferColorRegion(*/
                /*silk_ctx_curr->pixels,*/
                /*(vec2i){silk_ctx_curr->viewport_size.x, silk_ctx_curr->viewport_size.y},*/
                /*silk_ctx_curr->viewport_size.x,*/
                /*0);*/
        long time_start = get_system_ms();
        int size = silk_ctx_curr->viewport_size.x * silk_ctx_curr->viewport_size.y * 4;
        memset(silk_ctx_curr->buffer, 0, (size_t)size);
        /*memset(silk_ctx_curr->buffer, 0, (size_t)size);*/
        /*for (int i = 0; i < size; ++i) {*/
            /*silk_ctx_curr->buffer[i] = 0;*/
        /*}*/
        int size_pixels = silk_ctx_curr->viewport_size.x * silk_ctx_curr->viewport_size.y;
        for (int i = 0; i < size_pixels; ++i) {
            silk_ctx_curr->pixels[i] = YELLOW.rgba;
        }
        /*[>int size = silk_ctx_curr->viewport_size.x * silk_ctx_curr->viewport_size.y * 4;<]*/
        /*for (int i = 0; i < size; ++i) {*/
            /*silk_ctx_curr->buffer[i] = 0;*/
        /*}*/
        /*silkDrawRect(*/
                /*silk_ctx_curr->pixels,*/
                /*(vec2i){silk_ctx_curr->viewport_size.x, silk_ctx_curr->viewport_size.y},*/
                /*silk_ctx_curr->viewport_size.x,*/
                /*(vec2i) {0},*/
                /*(vec2i){silk_ctx_curr->viewport_size.x, silk_ctx_curr->viewport_size.y},*/
                /*YELLOW.rgba);*/
        silkDrawTextDefault(
                silk_ctx_curr->pixels,
                (vec2i){silk_ctx_curr->viewport_size.x, silk_ctx_curr->viewport_size.y},
                silk_ctx_curr->viewport_size.x, "HELLO", (vec2i){0}, 5, 1, BLACK.rgba);


        /*draw_all(ctx);*/
        silkDrawCircle(
                silk_ctx_curr->pixels,
                (vec2i){silk_ctx_curr->viewport_size.x, silk_ctx_curr->viewport_size.y},
                silk_ctx_curr->viewport_size.x,
                (vec2i) { 200 + (int)(((float)(ticks % 100)/100.0f) * 200.0f), 200},
                60,
                0xff0000ff
        );

        long time_end = get_system_ms();
        printfd("Drawing took %ld ms", time_end - time_start);
        time_start = time_end;

        /*x11_draw_texture(silk_ctx_curr->buffer, silk_ctx_curr->viewport_size);*/
        memcpy(x11ctx->bitmap->data, silk_buffer, (size_t)x11ctx->buf_len);
        x11_draw_texture();
        time_end = get_system_ms();
        printfd("X11 texture took %ld ms", time_end - time_start);

        /*draw_all(ctx);*/
        must_redraw ^= 1;

        /*silkClearPixelBufferColorRegion(silk_ctx_curr->pixels, (vec2i){silk_buffer_size.x, silk_buffer_size.y}, silk_buffer_size.x, 0x11AA0033);*/
        /*silkDrawCircle(*/
                /*silk_ctx_curr->pixels,*/
                /*(vec2i) { silk_buffer_size.x, silk_buffer_size.y },*/
                /*silk_buffer_size.x,*/
                /*(vec2i) { ticks * 2, 0},*/
                /*60,*/
                /*0xff0000ff*/
        /*);*/
        /*silkDrawRect(*/
                /*silk_ctx_curr->pixels,*/
                /*(vec2i) { silk_buffer_size.x, silk_buffer_size.y },*/
                /*silk_buffer_size.x,*/
                /*(vec2i) { 200, 0 },*/
                /*(vec2i) { 200, 200 },*/
                /*0xff0000ff*/
        /*);*/


        /* Poll for and process events */
        /*glfwPollEvents();*/
    }

    drawbuf_deinit();
    glfwTerminate();
    return 0;
}
