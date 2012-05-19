#include <string.h> /* memset */
#include <assert.h>
#include <math.h> /* tan, sin */
#include <stdio.h>

#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

int get_fb_resolution(uint32_t *width, uint32_t *height);
EGLSurface create_fb_surface(EGLDisplay display, EGLConfig config, uint32_t width, uint32_t height);
GLuint create_texture_from_shared_pixmap(EGLDisplay display, EGLint* global_image);

#define DISP_ID DISPMANX_ID_MAIN_LCD

typedef struct {
    uint32_t width, height;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int done;
    GLfloat roll;
    GLuint clientbuf;
} AppState;

static AppState g_inst, *g = &g_inst;

static const GLbyte quad[4*3] = {
    -10, -10, 10,
    10, -10, 10,
    -10, 10, 10,
    10, 10, 10,
};
static const GLfloat texCoords[4 * 2] = {
   0.f,  0.f,
   0.f,  4.f,
   4.f,  0.f,
   4.f,  4.f,
};

#ifndef M_PI
   #define M_PI 3.141592654
#endif

int start() {
    EGLBoolean result;
    EGLConfig config;
    EGLint num_config;
    
    g->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(g->display != EGL_NO_DISPLAY);

    result = eglInitialize(g->display, NULL, NULL);
    assert(result != EGL_FALSE);

    static const EGLint config_kw[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
    };
    result = eglChooseConfig(g->display, config_kw, &config, 1, &num_config);
    assert(result != EGL_FALSE);

    g->context = eglCreateContext(g->display, config, EGL_NO_CONTEXT, NULL);
    assert(g->context != EGL_NO_CONTEXT);

    assert(get_fb_resolution(&g->width, &g->height) == 0);

    g->surface = create_fb_surface(g->display, config, g->width, g->height);
    assert(g->surface != EGL_NO_SURFACE);

    result = eglMakeCurrent(g->display, g->surface, g->surface, g->context);
    assert(result != EGL_FALSE);

    // init viewport
    float nearp = 1.0f;
    float farp = 500.0f;
    float hht;
    float hwd;
    glViewport(0, 0, g->width, g->height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    hht = nearp * (float)tan(45.0 / 2.0 / 180.0 * M_PI);
    hwd = hht * (float)g->width / (float)g->height;
    glFrustumf(-hwd, hwd, -hht, hht, nearp, farp);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.f, 0.f, -50.f);
   
    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_BYTE, 0, quad );

    EGLint client[5];
    FILE* fd = fopen("global_image_handle.tmp", "r");
    fread(client, sizeof(EGLint), 5, fd);
    fclose(fd);
    
    glEnable(GL_TEXTURE_2D);

    g->clientbuf = create_texture_from_shared_pixmap(g->display, client);

    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

int stop() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear( GL_COLOR_BUFFER_BIT );
    eglSwapBuffers(g->display, g->surface);

    eglMakeCurrent(g->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface( g->display, g->surface );
    eglDestroyContext( g->display, g->context );
    eglTerminate( g->display );
}

int update() {
    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClear( GL_COLOR_BUFFER_BIT );

    glRotatef(g->roll, 0.0f, 0.0f, 1.0f);

    glEnable(GL_TEXTURE_2D);
    glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture(GL_TEXTURE_2D, g->clientbuf);
    
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

    eglSwapBuffers(g->display, g->surface);

    g->roll = -0.005;
}

int main() {
    bcm_host_init();
    memset(g, 0, sizeof(*g));
    start();
    while (!g->done) {
        update();
    }
    stop();
    return 0;
}
