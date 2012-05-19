#include <assert.h>
#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

#define EGL_PIXEL_FORMAT_ARGB_8888_PRE_BRCM 0
#define EGL_PIXEL_FORMAT_ARGB_8888_BRCM     1
#define EGL_PIXEL_FORMAT_XRGB_8888_BRCM     2
#define EGL_PIXEL_FORMAT_RGB_565_BRCM       3
#define EGL_PIXEL_FORMAT_A_8_BRCM           4
#define EGL_PIXEL_FORMAT_RENDER_GL_BRCM     (1 << 3)
#define EGL_PIXEL_FORMAT_RENDER_GLES_BRCM   (1 << 4)
#define EGL_PIXEL_FORMAT_RENDER_GLES2_BRCM  (1 << 5)
#define EGL_PIXEL_FORMAT_RENDER_VG_BRCM     (1 << 6)
#define EGL_PIXEL_FORMAT_RENDER_MASK_BRCM   0x78
#define EGL_PIXEL_FORMAT_VG_IMAGE_BRCM      (1 << 7)
#define EGL_PIXEL_FORMAT_GLES_TEXTURE_BRCM  (1 << 8)
#define EGL_PIXEL_FORMAT_GLES2_TEXTURE_BRCM (1 << 9)
#define EGL_PIXEL_FORMAT_TEXTURE_MASK_BRCM  0x380
#define EGL_PIXEL_FORMAT_USAGE_MASK_BRCM    0x3f8


EGLSurface create_shared_pixmap(EGLDisplay display, EGLConfig config, EGLint* global_image, int width, int height) {
    EGLint pixel_format = EGL_PIXEL_FORMAT_ARGB_8888_BRCM;
    EGLint rt;
    eglGetConfigAttrib(display, config, EGL_RENDERABLE_TYPE, &rt);

    if (rt & EGL_OPENGL_ES_BIT) {
        pixel_format |= EGL_PIXEL_FORMAT_RENDER_GLES_BRCM;
        pixel_format |= EGL_PIXEL_FORMAT_GLES_TEXTURE_BRCM;
    }
    if (rt & EGL_OPENGL_ES2_BIT) {
        pixel_format |= EGL_PIXEL_FORMAT_RENDER_GLES2_BRCM;
        pixel_format |= EGL_PIXEL_FORMAT_GLES2_TEXTURE_BRCM;
    }
    if (rt & EGL_OPENVG_BIT) {
        pixel_format |= EGL_PIXEL_FORMAT_RENDER_VG_BRCM;
        pixel_format |= EGL_PIXEL_FORMAT_VG_IMAGE_BRCM;
    }
    if (rt & EGL_OPENGL_BIT) {
        pixel_format |= EGL_PIXEL_FORMAT_RENDER_GL_BRCM;
    }
    global_image[0] = 0;
    global_image[1] = 0;
    global_image[2] = width;
    global_image[3] = height;
    global_image[4] = pixel_format;

    eglCreateGlobalImageBRCM(width, height, global_image[4], 0, width*4, global_image);

    EGLint attrs[] = {
        EGL_VG_COLORSPACE, EGL_VG_COLORSPACE_sRGB,
        EGL_VG_ALPHA_FORMAT, pixel_format & EGL_PIXEL_FORMAT_ARGB_8888_PRE_BRCM ? EGL_VG_ALPHA_FORMAT_PRE : EGL_VG_ALPHA_FORMAT_NONPRE,
        EGL_NONE
    };
    return eglCreatePixmapSurface(display, config, (EGLNativePixmapType)global_image, attrs);
}

GLuint create_texture_from_shared_pixmap(EGLDisplay display, EGLint* global_image) {
    assert (eglQueryGlobalImageBRCM(global_image, global_image+2));

    EGLImageKHR image = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR, (EGLClientBuffer)global_image, (void*)0/*NULL*/);
    assert (image != EGL_NO_IMAGE_KHR);

    GLuint tid;
    glGenTextures(1, &tid);
    glBindTexture(GL_TEXTURE_2D, tid);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
    /*
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    */

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //GL_CLAMP_TO_EDGE);

    eglDestroyImageKHR(display, image);

    return tid;
}
