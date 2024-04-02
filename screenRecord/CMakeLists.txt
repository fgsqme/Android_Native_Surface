cmake_minimum_required(VERSION 3.21)
project(screenRecord)

##################### 输出文件重定向 #####################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_SOURCE_DIR}/outputs/exec/${CMAKE_ANDROID_ARCH_ABI}/
) # 重定向输出产物(可执行文件)

##################### CMake头文件设置 #####################
include_directories(
        include
        include/tools
        ${CMAKE_SOURCE_DIR}/outputs/include
        ffmpeg/include
)

##################### CMake源文件设置 #####################
FILE(GLOB_RECURSE FILE_SOURCES # 遍历子目录下所有符合情况的源文件
        source/*.c*
)
##################### 设置三方库文件目录 #####################
link_directories(
        ffmpeg/lib/${ANDROID_ABI})

execute_process(COMMAND C:/Project/C++/Android_Native_Surface/hex/a.exe)


add_executable(nativeScreenRecord  # 生成可执行文件
        ${FILE_SOURCES} # 源文件
        screenRecord.cpp
)
add_executable(nativeRecordReceive # 生成可执行文件
        ${FILE_SOURCES} # 源文件
        recordReceive.cpp
)

add_executable(captureScreen # 生成可执行文件
        captureScreen.cpp
)

target_link_libraries(nativeScreenRecord PRIVATE EGL GLESv3 log android GLESv2 surface_static)
target_link_libraries(captureScreen PRIVATE EGL GLESv3 log android GLESv2 surface_static)

target_link_libraries(nativeRecordReceive PRIVATE
        surface_static
        log
        m
        z
        mediandk
        avformat
        avcodec
        avfilter
        swresample
        swscale
        avutil

)
##################### 连接库文件 #####################