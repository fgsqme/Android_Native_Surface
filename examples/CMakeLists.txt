cmake_minimum_required(VERSION 3.21)
# 设置NDK路径
set(NDK_PATH C:/MDK/android-ndk-r20b)

##################### Android设置 #####################
set(CMAKE_SYSTEM_NAME ANDROID) # 设置目标编译平台参数 Android
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_SYSTEM_VERSION 24) # 系统版本
set(ANDROID_PLATFORM 24) # 平台版本
set(ANDROID_SDK_TOOLS_VERSION 24) # 平台版本
set(ANDROID_ABI arm64-v8a) # 设置目标构架 armeabi-v7a arm64-v8a x86 x86_64
set(ANDROID_NDK ${NDK_PATH}) # 设置ndk路径
set(CMAKE_TOOLCHAIN_FILE ${NDK_PATH}/build/cmake/android.toolchain.cmake)
set(ANDROID_SDK_ROOT ${NDK_PATH})

project(examples)
set(CMAKE_CXX_STANDARD 17)

##################### CMake头文件设置 #####################
# 遍历子目录下所有符合情况的头文件
include_directories( # 设置全局头文件目录 使其他源码文件可在任意目录<头文件.h>
        include
)

##################### CMake源文件设置 #####################
add_executable(nativeSurface # 生成可执行文件
        demo.cpp
)

add_library(surface_static STATIC IMPORTED)
set_target_properties(surface_static PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/${CMAKE_ANDROID_ARCH_ABI}/libsurface_static.a)
target_link_libraries(nativeSurface PRIVATE EGL GLESv3 log android GLESv2 surface_static)
