//
// Created by fgsqme on 2022/9/9.
//

#include "extern_function.h"
#include "aosp/native_surface_9.h"
#include "aosp/native_surface_10.h"
#include "aosp/native_surface_11.h"
#include "aosp/native_surface_12.h"
#include "aosp/native_surface_12_1.h"
#include "aosp/native_surface_13.h"
#include "aosp/native_surface_14.h"
#include "aosp/dev.h"

// 方法指针
struct FuncPointer {
    void *func_createNativeWindow;
    //    更多可选参数
    void *func_more_createNativeWindow;
    void *func_getDisplayInfo;
//    void *func_setSurfaceWH;
    void *func_runRecord;
    void *func_stopRecord;
    void *func_initRecord;
    void *func_getRecordNativeWindow;
    void *func_captureScreen;
};

// 动态库方案
static void *handle;
static FuncPointer funcPointer;

ExternFunction::ExternFunction() {
    if (!handle) {
//        handle = dlblob(&native_surface_test, sizeof(native_surface_test)); // 测试
        printf("android api level:%d\n", get_android_api_level());
        if (get_android_api_level() == 34) { // 安卓14支持
            exec_native_surface("settings put global block_untrusted_touches 0");
#ifdef __aarch64__
            handle = dlblob(&native_surface_14_64, sizeof(native_surface_14_64)); // 64位支持
#else
            handle = dlblob(&native_surface_14_32, sizeof(native_surface_14_32)); // 32位支持 <<-- 其实很没必要 未测试
#endif
        } else if (get_android_api_level() == 33) { // 安卓13支持
            exec_native_surface("settings put global block_untrusted_touches 0");
#ifdef __aarch64__
            handle = dlblob(&native_surface_13_64, sizeof(native_surface_13_64)); // 64位支持
#else
            handle = dlblob(&native_surface_13_32, sizeof(native_surface_13_32)); // 32位支持 <<-- 其实很没必要 未测试
#endif
        } else if (get_android_api_level() == 32) { // 安卓12.1支持
            exec_native_surface("settings put global block_untrusted_touches 0");
#ifdef __aarch64__
            handle = dlblob(&native_surface_12_1_64, sizeof(native_surface_12_1_64)); // 64位支持
#else
            handle = dlblob(&native_surface_12_1_32, sizeof(native_surface_12_1_32)); // 32位支持 <<-- 其实很没必要 未测试
#endif
        } else if (get_android_api_level() == 31) { // 安卓12支持
            exec_native_surface("settings put global block_untrusted_touches 0");
#ifdef __aarch64__
            handle = dlblob(&native_surface_12_64, sizeof(native_surface_12_64)); // 64位支持
#else
            handle = dlblob(&native_surface_12_32, sizeof(native_surface_12_32)); // 32位支持 <<-- 其实很没必要 未测试
#endif
        } else if (get_android_api_level() == 30) { // 安卓11支持
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
        funcPointer.func_createNativeWindow = dlsym(handle, "_Z18createNativeWindowPKcjjb");
        funcPointer.func_more_createNativeWindow = dlsym(handle, "_Z18createNativeWindowPKcjjjjb");

        // 获取屏幕信息
        funcPointer.func_getDisplayInfo = dlsym(handle, "_Z14getDisplayInfov");
//        funcPointer.func_setSurfaceWH = dlsym(handle, "_Z12setSurfaceWHjj");
        funcPointer.func_initRecord = dlsym(handle, "_Z10initRecordPKcfjj");
        funcPointer.func_runRecord = dlsym(handle, "_Z9runRecordPbPFvPhmE");
        funcPointer.func_stopRecord = dlsym(handle, "_Z10stopRecordv");
        funcPointer.func_getRecordNativeWindow = dlsym(handle, "_Z21getRecordNativeWindowv");
        funcPointer.func_captureScreen = dlsym(handle, "_Z13captureScreenhhPNSt3__16vectorIhNS_9allocatorIhEEEE");
    }

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
                                   bool author) {
    return ((ANativeWindow *(*)(
            const char *, uint32_t, uint32_t, bool))
            (funcPointer.func_createNativeWindow))(surface_name, screen_width, screen_height, author);
}

/**
 * (更多可选参数)创建 native surface
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
                                   uint32_t format, uint32_t flags, bool author) {
    return ((ANativeWindow *(*)(
            const char *, uint32_t, uint32_t, uint32_t, uint32_t, bool))
            (funcPointer.func_more_createNativeWindow))(surface_name, screen_width, screen_height, format, flags,
                                                        author);
}

/**
 * 获取屏幕宽高以及旋转状态
 */
MDisplayInfo ExternFunction::getDisplayInfo() {
    return ((MDisplayInfo (*)()) (funcPointer.func_getDisplayInfo))();
}

/**
 * 设置画布大小(没啥用暂时屏蔽)
 * @param width
 * @param height
 */
//void ExternFunction::setSurfaceWH(uint32_t width, uint32_t height) {
//    return ((void (*)(uint32_t, uint32_t)) (funcPointer.func_setSurfaceWH))(width, height);
//}

/**
 * h264 录屏
 * 录屏初始化
 * @param bitRate 码率 <= 200M
 * @param fps (设置了没用就很奇怪)
 * @param videoWidth 录制视频宽度，设置0为默认屏幕宽
 * @param videoHeight 录制视频高度 设置0为默认屏幕高
 */
void ExternFunction::initRecord(const char *bitRate, float fps,
                                uint32_t videoWidth, uint32_t videoHeight) {
    ((void (*)(const char *, float, uint32_t, uint32_t)) (funcPointer.func_initRecord))
            (bitRate, fps, videoWidth, videoHeight);
}

/**
 * 获取录屏window(有bug暂时无用)
 * @return
 */
ANativeWindow *ExternFunction::getRecordNativeWindow() {
    return ((ANativeWindow *(*)()) (funcPointer.func_getRecordNativeWindow))();
}

/**
 * 开始录屏
 * @param gStopRequested
 * @param callback
 */
void ExternFunction::runRecord(bool *flag, void (*callback)(uint8_t *, size_t)) {
    ((void (*)(bool *, void(uint8_t *, size_t))) (funcPointer.func_runRecord))(flag, callback);
}

/**
 * 录屏结束调用
 */
void ExternFunction::stopRecord() {
    ((void (*)()) (funcPointer.func_stopRecord))();
}

/**
 * 截屏
 * @param type  输出图像类型 0:png, 1:jpg
 * @param quality 输出质量 0 - 100
 * @param buff buff
 * @return 是否成功
 */
int ExternFunction::captureScreen(uint8_t type, uint8_t quality, vector<uint8_t> *buff) {
    return ((int (*)(uint8_t, uint8_t, vector<uint8_t> *)) (funcPointer.func_captureScreen))(type, quality, buff);
}