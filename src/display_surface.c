/* Copyright (c) 2012 Cheery
 *
 * See the file license.txt for copying permission. */
#include "bcm_host.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

#define DISPLAY_ID DISPMANX_ID_MAIN_LCD /*0*/


int get_fb_resolution(uint32_t *width, uint32_t *height){
    int success = graphics_get_display_size(DISPLAY_ID, width, height);
    return success >= 0 ? 0 : -1;
}

EGLSurface create_fb_surface(EGLDisplay display, EGLConfig config, uint32_t width, uint32_t height) {
    static EGL_DISPMANX_WINDOW_T nativewindow;

    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;

    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.width = width;
    dst_rect.height = height;

    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = width << 16;
    src_rect.height = height << 16;

    dispman_display = vc_dispmanx_display_open(DISPLAY_ID);
    dispman_update = vc_dispmanx_update_start(0);
    dispman_element = vc_dispmanx_element_add(
        dispman_update, dispman_display,
        0/*layer*/, &dst_rect, 0/*src*/,
        &src_rect, DISPMANX_PROTECTION_NONE, 0/*alpha*/, 0/*clamp*/, 0/*transform*/);

    vc_dispmanx_update_submit_sync(dispman_update);

    nativewindow.element = dispman_element;
    nativewindow.width = width;
    nativewindow.height = height;
    return eglCreateWindowSurface(display, config, &nativewindow, 0/*NULL*/);
}
