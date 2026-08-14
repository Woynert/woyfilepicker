/*
   Some relevant docs:
   https://linux.die.net/man/3/xshmattach

   Notes:
   * Uses double buffer and swap to prevent tearing.
   * If you see tearing that means you're swapping too fast (uncapped FPS).
     You've hit the limit of your x11 server processing speed. Even
     if we wait for "finish reading" events you'll still have reduced FPS
     because of the wait.
   * I don't think is worth waiting for the x11 server to notify it has
     finished reading the buffer. Since we're using GLFW we don't have
     the guarantee it won't eat it before we can detect it. Also "detecting" it
     seems to be slow anyway.
    */

#ifndef X11BACK_H
#define X11BACK_H

#include "portable_utils.h"
#include "la.h"
#include "silk.h"
#include <X11/Xlib.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <X11/Xlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

#define SHM_INVALID ((void *)-1)

typedef struct x11back_t {
    Display *display;
    Window window;
    GC gc;
    int screen;
    XImage *bitmap;
    XVisualInfo vi;
    XShmSegmentInfo shminfo;
    int buf_len;
} x11back_t;

x11back_t X11CTX__ = { 0 };
x11back_t *x11ctx = &X11CTX__;


/// @Return the buffer you shall use this frame.
char *x11_swap_buffer(void) {
    char *prev = x11ctx->bitmap->data;
    x11ctx->bitmap->data = x11ctx->bitmap->data == x11ctx->shminfo.shmaddr ?
        x11ctx->shminfo.shmaddr + x11ctx->buf_len/2 :
        x11ctx->shminfo.shmaddr;
    return prev;
}

int x11_ensure_size(V2i target_size) {
    printfd("Requested "V2i_Fmt, V2i_Arg(target_size));

    // ↓↓↓ Multiply by 2 for double buffer.
    int target_buf_len = target_size.x * target_size.y * (int)sizeof(pixel) * 2;

    if (target_buf_len > x11ctx->buf_len) {
        if (x11ctx->shminfo.shmaddr != SHM_INVALID) { // Free.
            shmdt(x11ctx->shminfo.shmaddr);
            shmctl(x11ctx->shminfo.shmid, IPC_RMID, NULL);
        }

        x11ctx->shminfo.shmid = shmget(IPC_PRIVATE, (size_t)target_buf_len, IPC_CREAT|0777);
        if (x11ctx->shminfo.shmid == -1) { printfd("ERR"); return -1; }

        x11ctx->shminfo.shmaddr = (char*)shmat(x11ctx->shminfo.shmid, 0, 0);
        if (x11ctx->shminfo.shmaddr == SHM_INVALID) {
            printfd("ERR: Couldn't allocate.");
            shmdt(x11ctx->shminfo.shmaddr);
            return -1;
        }
        x11ctx->buf_len = target_buf_len;

        x11ctx->shminfo.readOnly = False;
        int err = XShmAttach(x11ctx->display, &x11ctx->shminfo);
        if (!err) {
            printfd("ERR: Couldn't attach.");
            shmdt(x11ctx->shminfo.shmaddr);
            shmctl(x11ctx->shminfo.shmid, IPC_RMID, NULL);
            return -1;
        }
    }

    x11ctx->bitmap->data = x11ctx->shminfo.shmaddr;
    x11ctx->bitmap->width = target_size.x;
    x11ctx->bitmap->height = target_size.y;
    x11ctx->bitmap->bytes_per_line = target_size.x * (int)(sizeof(pixel));
    printfd("Success.");
    return 0;
}

int x11back_init(GLFWwindow* glfw_window) {
    x11ctx->display   = glfwGetX11Display();
    x11ctx->window    = glfwGetX11Window(glfw_window);
    x11ctx->gc        = XCreateGC(x11ctx->display, x11ctx->window, 0, NULL);
    x11ctx->screen    = DefaultScreen(x11ctx->display);
    x11ctx->vi.visual = DefaultVisual(x11ctx->display, DefaultScreen(x11ctx->display));
    x11ctx->shminfo.shmaddr = (char*)SHM_INVALID;

    int err = XMatchVisualInfo(x11ctx->display, DefaultScreen(x11ctx->display),
        DefaultDepth(x11ctx->display, x11ctx->screen), TrueColor, &x11ctx->vi);
    if (!err) { printfd("ERR"); return -1; }

    x11ctx->bitmap = XShmCreateImage(
        x11ctx->display, x11ctx->vi.visual, (unsigned int)x11ctx->vi.depth,
        ZPixmap, NULL, &x11ctx->shminfo, 500, 500);
    if (!x11ctx->bitmap) { printfd("ERR"); return -1; }

    return x11_ensure_size((V2i){{500, 500}});
}

char *x11_get_buffer(void) {
    return (x11ctx->bitmap->data == SHM_INVALID
        || x11ctx->bitmap->data == NULL) ?
        NULL : x11ctx->bitmap->data;
}

void x11_draw_texture(void) {
    XShmPutImage(x11ctx->display, x11ctx->window, x11ctx->gc, x11ctx->bitmap,
            0, 0, 0, 0,
            (unsigned int)x11ctx->bitmap->width, (unsigned int)x11ctx->bitmap->height, false
    );
}

#endif
