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

project(NativeSurface)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(libsurface)
add_subdirectory(screenRecord)


