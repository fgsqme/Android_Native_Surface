//
// Created by user on 2022/9/9.
//

#include "extern_function.h"
#include "native_surface/aosp/native_surface_9.h"
#include "native_surface/aosp/native_surface_10.h"
#include "native_surface/aosp/native_surface_11.h"
#include "native_surface/aosp/native_surface_12.h"
#include "native_surface/aosp/native_surface_12_1.h"
#include "native_surface/aosp/native_surface_13.h"

static void *handle;// 动态库方案

ExternFunction::ExternFunction() {
    printf("android api level:%d\n", get_android_api_level());
    if (!handle) {
        if (get_android_api_level() == 33) { // 安卓13支持
            exec_native_surface("settings put global block_untrusted_touches 0");
#ifdef __aarch64__
            handle = dlblob(&native_surface_13_64, sizeof(native_surface_13_64)); // 64位支持
#else
            handle = dlblob(&native_surface_13_32, sizeof(native_surface_13_32)); // 32位支持 <<-- 其实很没必要 未测试
#endif
        } else if (get_android_api_level() == /*__ANDROID_API_S__*/ 32) { // 安卓12.1支持
            exec_native_surface("settings put global block_untrusted_touches 0");
#ifdef __aarch64__
            handle = dlblob(&native_surface_12_1_64, sizeof(native_surface_12_1_64)); // 64位支持
#else
            handle = dlblob(&native_surface_12_1_32, sizeof(native_surface_12_1_32)); // 32位支持 <<-- 其实很没必要 未测试
#endif
        } else if (get_android_api_level() == /*__ANDROID_API_S__*/ 31) { // 安卓12支持
            exec_native_surface("settings put global block_untrusted_touches 0");
#ifdef __aarch64__
            handle = dlblob(&native_surface_12_64, sizeof(native_surface_12_64)); // 64位支持
#else
            handle = dlblob(&native_surface_12_32, sizeof(native_surface_12_32)); // 32位支持 <<-- 其实很没必要 未测试
#endif
//            funcPointer.func_more_createNativeWindow = dlsym(handle, "_Z18createNativeWindowPKcjjjjb");
        } else if (get_android_api_level() == /*__ANDROID_API_R__*/ 30) { // 安卓11支持
#ifdef __aarch64__
            handle = dlblob(&native_surface_11_64, sizeof(native_surface_11_64)); // 64位支持
#else
            handle = dlblob(&native_surface_11_32, sizeof(native_surface_11_32)); // 32位支持 <<-- 其实很没必要 未测试
#endif
        } else if (get_android_api_level() == __ANDROID_API_Q__) { // 安卓10支持
#ifdef __aarch64__
            handle = dlblob(&native_surface_10_64, sizeof(native_surface_10_64)); // 64位支持
#else
            handle = dlblob(&native_surface_10_32, sizeof(native_surface_10_32)); // 32位支持 <<-- 其实很没必要 未测试
#endif
        } else if (get_android_api_level() == __ANDROID_API_P__) { // 安卓9支持
#ifdef __aarch64__
            handle = dlblob(&native_surface_9_64, sizeof(native_surface_9_64)); // 64位支持
#else
            handle = dlblob(&native_surface_9_32, sizeof(native_surface_9_32)); // 32位支持 <<-- 其实很没必要 未测试
#endif
        } else {
            printf("Sorry, level:%d Don't Support~\n", get_android_api_level());
            exit(0);
        }
    }

    funcPointer.func_createNativeWindow = dlsym(handle, "_Z18createNativeWindowPKcjjb");
    // 获取屏幕信息
    funcPointer.func_getDisplayInfo = dlsym(handle, "_Z14getDisplayInfov");
    funcPointer.func_setSurfaceWH = dlsym(handle, "_Z12setSurfaceWHjj");
    funcPointer.func_initRecord = dlsym(handle, "_Z10initRecordv");
    funcPointer.func_runRecord = dlsym(handle, "_Z9runRecordPbPFvPhmE");
    funcPointer.func_stopRecord = dlsym(handle, "_Z10stopRecordv");

}

/**
 * 创建 native surface
 * @param surface_name 创建名称
 * @param screen_width 创建宽度
 * @param screen_height 创建高度
 * @param author 是否打印作者信息
 * @return
 */
ANativeWindow *
ExternFunction::createNativeWindow(const char *surface_name, uint32_t screen_width, uint32_t screen_height,
                                   bool author) const {
    return ((ANativeWindow *(*)(
            const char *, uint32_t, uint32_t, bool))
            (funcPointer.func_createNativeWindow))(surface_name, screen_width, screen_height, author);
}

/**
 * (更多可选参数_暂时只支持安卓12)创建 native surface
 * @param surface_name 创建名称
 * @param screen_width 创建宽度
 * @param screen_height 创建高度
 * @param format format
 * @param flags flags
 * @param author 是否打印作者信息
 * @return
 */
ANativeWindow *
ExternFunction::createNativeWindow(const char *surface_name, uint32_t screen_width, uint32_t screen_height,
                                   uint32_t format, uint32_t flags, bool author) const {
    return ((ANativeWindow *(*)(
            const char *, uint32_t, uint32_t, uint32_t, uint32_t, bool))
            (funcPointer.func_more_createNativeWindow))(surface_name, screen_width, screen_height, format, flags,
                                                        author);
}

/**
 * 获取屏幕宽高以及旋转状态
 */
MDisplayInfo ExternFunction::getDisplayInfo() const {
    return ((MDisplayInfo (*)()) (funcPointer.func_getDisplayInfo))();
}

void ExternFunction::setSurfaceWH(uint32_t width, uint32_t height) const {
    return ((void (*)(uint32_t, uint32_t)) (funcPointer.func_setSurfaceWH))(width, height);
}

/**
 * 录屏初始化(安卓12)
 */
void ExternFunction::initRecord() const {
    ((void (*)()) (funcPointer.func_initRecord))();
}

/**
 * 开始录屏(安卓12)
 * @param gStopRequested
 * @param callback
 */
void ExternFunction::runRecord(bool *gStopRequested, void (*callback)(uint8_t *, size_t)) const {
    ((void (*)(bool *, void(uint8_t *, size_t))) (funcPointer.func_runRecord))(gStopRequested, callback);
}
/**
 * 录屏结束调用
 */
void ExternFunction::stopRecord() const {
    ((void (*)()) (funcPointer.func_stopRecord))();
}