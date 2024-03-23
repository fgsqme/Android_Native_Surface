#include "draw.h"


//
// Created by 泓清 on 2022/8/26.
//

#include "draw.h"
#include <font/Font.h>

// User libs

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_android.h"

#include <EGL/egl.h>
#include <dlfcn.h>
#include <cstdio>
#include <android/native_window.h>
#include <GLES3/gl32.h>


// Var
EGLDisplay display = EGL_NO_DISPLAY;
EGLConfig config;
EGLSurface surface = EGL_NO_SURFACE;
EGLContext context = EGL_NO_CONTEXT;


bool DrawOpenGL::initDraw(int flags, bool log) {
    screen_config();
    orientation = displayInfo.orientation;
    return initDraw(displayInfo.width, displayInfo.height, flags, log);

}

bool DrawOpenGL::initDraw(uint32_t _screen_x, uint32_t _screen_y, int flags, bool log) {
    width = _screen_x;
    height = _screen_y;
    if (g_Initialized) {
        return true;
    }
    int f = 0;
    // 解决安卓14触摸失效问题
    if (get_android_api_level() >= 33) {
        f |= 0x2000;
    }
    if (flags > 0) {
        f |= flags;
    }
    native_window = externFunction.createNativeWindow("Ssage",
                                                      _screen_x, _screen_y, 1, f, false);
    ANativeWindow_acquire(native_window);
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        printf("eglGetDisplay error=%u\n", glGetError());
        return false;
    }
    if (log) {
        printf("eglGetDisplay ok\n");
    }
    if (eglInitialize(display, 0, 0) != EGL_TRUE) {
        printf("eglInitialize error=%u\n", glGetError());
        return false;
    }
    if (log) {
        printf("eglInitialize ok\n");
    }
    EGLint num_config = 0;
    const EGLint attribList[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_BLUE_SIZE, 5,   //-->delete
            EGL_GREEN_SIZE, 6,  //-->delete
            EGL_RED_SIZE, 5,    //-->delete
            EGL_BUFFER_SIZE, 32,  //-->new field
            EGL_DEPTH_SIZE, 16,
            EGL_STENCIL_SIZE, 8,
            EGL_NONE
    };
    const EGLint attrib_list[] = {
            EGL_CONTEXT_CLIENT_VERSION,
            3,
            EGL_NONE
    };
    if (log) {
        printf("num_config=%d\n", num_config);
    }
    if (eglChooseConfig(display, attribList, &config, 1, &num_config) != EGL_TRUE) {
        printf("eglChooseConfig  error=%u\n", glGetError());
        return false;
    }
    if (log) {
        printf("eglChooseConfig ok\n");
    }
    EGLint egl_format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &egl_format);
    ANativeWindow_setBuffersGeometry(native_window, 0, 0, egl_format);
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, attrib_list);
    if (context == EGL_NO_CONTEXT) {
        printf("eglCreateContext  error = %u\n", glGetError());
        return false;
    }
    if (log) {
        printf("eglCreateContext ok\n");
    }
    surface = eglCreateWindowSurface(display, config, native_window, nullptr);
    if (surface == EGL_NO_SURFACE) {
        printf("eglCreateWindowSurface  error = %u\n", glGetError());
        return false;
    }
    if (log) {
        printf("eglCreateWindowSurface ok\n");
    }
    if (!eglMakeCurrent(display, surface, surface, context)) {
        printf("eglMakeCurrent  error = %u\n", glGetError());
        return false;
    }
    if (log) {
        printf("eglMakeCurrent ok\n");
        printf("createNativeWindow ok\n");
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.DisplaySize = ImVec2((float) _screen_x, (float) _screen_y);
    ImGui::StyleColorsDark();
    ImGui_ImplAndroid_Init(native_window);
    ImGui_ImplOpenGL3_Init("#version 300 es");
    ImFontConfig font_cfg;
    font_cfg.SizePixels = 22.0f;
    io.Fonts->AddFontDefault(&font_cfg);
//    ImFontConfig font_cfg;
//    io.Fonts->AddFontFromMemoryTTF((void*)OPPOSans_H, OPPOSans_H_size, 22.0f, &font_cfg);
    ImGui::GetStyle().ScaleAllSizes(3.0f);
    g_Initialized = true;
    return true;
}

void DrawOpenGL::setDisableRecordState(bool b) {
    if (disableRecord != b) {
        shutdown();
        initDraw();
    }
}

void DrawOpenGL::screen_config() {
    displayInfo = externFunction.getDisplayInfo();
}


bool DrawOpenGL::drawBegin() {
    screen_config();
    bool flag = false;
    if (orientation != displayInfo.orientation) {
        orientation = displayInfo.orientation;
        printf("width: %d height: %d height: %d\n", displayInfo.width, displayInfo.height, displayInfo.orientation);
        shutdown();
        initDraw();
        flag = true;
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();
    return flag;
}

void DrawOpenGL::drawEnd() {
    ImGuiIO &io = ImGui::GetIO();
    glViewport(0.0f, 0.0f, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT); // GL_DEPTH_BUFFER_BIT
    glFlush();
    if (display == EGL_NO_DISPLAY) {
        return;
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    eglSwapBuffers(display, surface);
}


void DrawOpenGL::shutdown() {
    if (!g_Initialized) {
        return;
    }
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();
    if (display != EGL_NO_DISPLAY) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context != EGL_NO_CONTEXT) {
            eglDestroyContext(display, context);
        }
        if (surface != EGL_NO_SURFACE) {
            eglDestroySurface(display, surface);
        }
        eglTerminate(display);
    }
    display = EGL_NO_DISPLAY;
    context = EGL_NO_CONTEXT;
    surface = EGL_NO_SURFACE;
    ANativeWindow_release(native_window);
    g_Initialized = false;
}
