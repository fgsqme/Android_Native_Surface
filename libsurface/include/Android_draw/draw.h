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

class Draw {
public:
    ANativeWindow *native_window;
    ExternFunction externFunction;
    bool g_Initialized = false;
    uint32_t orientation = 0;
    bool disableRecord = false;
    MDisplayInfo displayInfo;
    int width;
    int height;


public:
    virtual ~Draw() = default;

    // 初始化
    virtual bool initDraw(uint32_t _screen_x, uint32_t _screen_y, int flags = 0, bool log = false) = 0;

    virtual bool initDraw(int flags = 0,bool log = false) = 0;

    // 跳过 / 不跳过 录屏
    virtual void setDisableRecordState(bool b) = 0;

    // 获取屏幕当前绝对宽高和旋转状态
    virtual void screen_config() = 0;

    virtual bool drawBegin() = 0;

    virtual void drawEnd() = 0;

    virtual void shutdown() = 0;

};

class DrawOpenGL : public Draw {
    bool initDraw(uint32_t _screen_x, uint32_t _screen_y, int flags = 0, bool log = false) override;

    bool initDraw(int flags = 0,bool log = false) override;

    void screen_config() override;

    void setDisableRecordState(bool b) override;

    bool drawBegin() override;

    void drawEnd() override;

    void shutdown() override;
};

class DrawVulkan : public Draw {
    bool initDraw(uint32_t _screen_x, uint32_t _screen_y, int flags = 0, bool log = false) override;

    bool initDraw(int flags = 0,bool log = false) override;

    void screen_config() override;

    void setDisableRecordState(bool b) override;

    bool drawBegin() override;

    void drawEnd() override;

    void shutdown() override;
};

#endif //NATIVESURFACE_DRAW_H
