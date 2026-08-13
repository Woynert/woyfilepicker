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


void glfw_mouse_callback(GLFWwindow* w, int button, int action, int mods) {
    winput_glfw_mouse_button_callback(w, button, action, mods);
}
void glfw_scroll_callback(GLFWwindow* w, double xoffset, double yoffset) {
    winput_glfw_scroll_callback(w, xoffset, yoffset);
}
void glfw_cursor_pos_callback(GLFWwindow *w, double xpos, double ypos) {
    winput_glfw_cursor_pos_callback(w, xpos, ypos);
}
void glfw_key_callback(GLFWwindow* w, int key, int scancode, int action, int mods) {
    kinput_glfw_key_callback(w, key, scancode, action, mods);
}
void glfw_window_size_callback (GLFWwindow *w, int width, int height) {
    Ctx *ctx = (Ctx*)glfwGetWindowUserPointer(w);
    ctx->window_size = (V2i) {{ int_max(2, width), int_max(2, height) }};
    silk_resize(&ctx->silk_ctx, ctx->window_size);
    printfd("Resized "V2i_Fmt, V2i_Arg(ctx->window_size));
}
void hook_glfw_callbacks(GLFWwindow* w, Ctx *ctx) {
    glfwSetWindowUserPointer(w, ctx);
    glfwSetCursorPosCallback(w, glfw_cursor_pos_callback);
    glfwSetMouseButtonCallback(w, glfw_mouse_callback);
    glfwSetScrollCallback(w, glfw_scroll_callback);
    glfwSetKeyCallback(w, glfw_key_callback);
    glfwSetWindowSizeCallback(w, glfw_window_size_callback);
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
    /*u8* silk_buffer = (u8*)malloc((size_t)(silk_buffer_size.x * silk_buffer_size.y) * 4);*/

    /*SilkCtx silk_ctx = { 0 };*/
    silk_ctx_curr = &ctx->silk_ctx;
    silk_ctx_init(silk_ctx_curr);
    silk_resize(silk_ctx_curr, ctx->window_size);
    /*silk_resize(silk_ctx_curr, (V2i) {{*/
        /*ctx->window_size.x),*/
        /*int_max(SILK_PIXELBUFFER_HEIGHT, ctx->window_size.y)*/
    /*}});*/

    x11back_init(window);
    /*x11back_init(window, (V2i){{ silk_buffer_size.x, silk_buffer_size.y }});*/
    drawbuf_init();
    drawbuf__DrawRectCallback = &silk_DrawRectCallback;


    double GLFW_EVENT_TIMEOUT_SECS = 1;
    int ticks = 0;
    while (!glfwWindowShouldClose(window))
    {
        glfwWaitEventsTimeout(GLFW_EVENT_TIMEOUT_SECS);
        printf("loop %d\n", ticks++);

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

        draw_all(ctx);

        x11_draw_texture(silk_ctx_curr->buffer, silk_ctx_curr->viewport_size);

        /* Poll for and process events */
        glfwPollEvents();
    }

    drawbuf_deinit();
    glfwTerminate();
    return 0;
}
