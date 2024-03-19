cmake_minimum_required(VERSION 3.21)
project(examples)

##################### 输出文件重定向 #####################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_SOURCE_DIR}/outputs/exec/${CMAKE_ANDROID_ARCH_ABI}/
) # 重定向输出产物(可执行文件)
##################### CMake头文件设置 #####################
# 遍历子目录下所有符合情况的头文件
include_directories( # 设置全局头文件目录 使其他源码文件可在任意目录<头文件.h>
        ${CMAKE_SOURCE_DIR}/outputs/include
)
##################### CMake源文件设置 #####################
add_executable(nativeSurface # 生成可执行文件
        surface.cpp
)
target_link_libraries(nativeSurface PRIVATE surface_static)
