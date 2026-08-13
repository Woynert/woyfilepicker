#ifndef X11BACK_H
#define X11BACK_H

#include "portable_utils.h"
#include "la.h"
#include "silk.h"
#include <X11/Xlib.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

typedef struct x11back_t {
    Display *display;
    Window window;
    GC gc;
    int screen;
    XImage *bitmap;
    XVisualInfo vi;
} x11back_t;

x11back_t X11CTX__ = { 0 };
x11back_t *x11ctx = &X11CTX__;

int x11back_init(GLFWwindow* glfw_window) {
    // https://medium.com/@colleagueriley/rgfw-under-the-hood-software-rendering-82f54a6da419
    x11ctx->display = glfwGetX11Display();
    x11ctx->window = glfwGetX11Window(glfw_window);
    x11ctx->gc = XCreateGC(x11ctx->display, x11ctx->window, 0, NULL);
    x11ctx->screen = DefaultScreen(x11ctx->display);
    x11ctx->vi.visual = DefaultVisual(x11ctx->display, DefaultScreen(x11ctx->display));
    int err = XMatchVisualInfo(x11ctx->display, DefaultScreen(x11ctx->display),
            DefaultDepth(x11ctx->display, x11ctx->screen), TrueColor, &x11ctx->vi);
    if (!err) { printfd("ERR"); return -1; }
    x11ctx->bitmap = XCreateImage(
        x11ctx->display, x11ctx->vi.visual, (unsigned int)x11ctx->vi.depth,
        ZPixmap, 0, NULL, 0, 0, 32, 0);
    return 0;
}

void x11_draw_texture(char *buffer, V2i size) {
    if (size.x != x11ctx->bitmap->width || size.y != x11ctx->bitmap->height) {
        x11ctx->bitmap->width = size.x;
        x11ctx->bitmap->height = size.y;
        x11ctx->bitmap->bytes_per_line = size.x * (int)(sizeof(pixel));
        //x11ctx->bitmap->data = NULL;
        //XDestroyImage(x11ctx->bitmap);
        //x11ctx->bitmap = XCreateImage(
            //x11ctx->display, x11ctx->vi.visual, (unsigned int)x11ctx->vi.depth,
            //ZPixmap, 0, NULL, (unsigned int)size.x, (unsigned int)size.y, 32, 0);
    }
    x11ctx->bitmap->data = buffer;
    XPutImage(x11ctx->display, x11ctx->window, x11ctx->gc, x11ctx->bitmap,
            0, 0, 0, 0, (unsigned int)size.x, (unsigned int)size.y);
}

#endif
