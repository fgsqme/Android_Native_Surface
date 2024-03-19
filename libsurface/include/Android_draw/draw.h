//
// Created by 泓清 on 2022/8/26.
//
#ifndef NATIVESURFACE_DRAW_H
#define NATIVESURFACE_DRAW_H
// System libs
#include <iostream>
#include <thread>
#include <chrono>

// User libs
//#include <touch.h>
#include "extern_function.h"


// namespace
using namespace std;
using namespace std::chrono_literals;
//extern EGLDisplay display;
//extern EGLConfig config;
//extern EGLSurface surface;
//extern ANativeWindow *native_window;
extern ExternFunction externFunction;
//extern EGLContext context;
// 屏幕信息
extern MDisplayInfo displayInfo;
extern bool g_Initialized;

// Func
bool init_egl(uint32_t _screen_x, uint32_t _screen_y, bool log = false);

bool initDraw(uint32_t _screen_x, uint32_t _screen_y, bool log = false);

bool initDraw(bool log = false);

bool ImGui_init();

void screen_config();

void drawBegin();

void drawEnd();

void shutdown();

#endif //NATIVESURFACE_DRAW_H
