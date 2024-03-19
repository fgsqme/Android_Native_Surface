cmake_minimum_required(VERSION 3.21)
project(libsurface)

##################### 输出文件重定向 #####################
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/outputs/lib/${CMAKE_ANDROID_ARCH_ABI}/)
##################### CMake头文件设置 #####################
# 遍历子目录下所有符合情况的头文件
FILE(GLOB_RECURSE FILE_INCLUDES include/*.h*)
include_directories( # 设置全局头文件目录 使其他源码文件可在任意目录<头文件.h>
        include
        private_include
        include/imgui
        include/Android_touch
        include/Android_shm
        include/native_surface
        include/Android_draw
        libhv_static/include
        )
##################### CMake头文件设置 #####################

##################### CMake源文件设置 #####################
FILE(GLOB_RECURSE FILE_SOURCES # 遍历子目录下所有符合情况的源文件
        source/*.c*
        )
FILE(GLOB_RECURSE GLOB_IMGUI_RECURSE # 遍历子目录下所有符合情况的源文件
        source/imgui/*.c*
        )


##################### CMake源文件设置 #####################
add_library(surface_static
        STATIC
        ${FILE_INCLUDES}
        ${FILE_SOURCES}
        ${GLOB_IMGUI_RECURSE}
)


add_library(hv_static STATIC IMPORTED)
set_target_properties(hv_static PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libhv_static/lib/${ANDROID_ABI}/libhv_static.a)

target_link_libraries(surface_static PRIVATE EGL GLESv3 log android GLESv2 m dl
        mediandk
        hv_static
)


#####################复制头文件#######################
# 设置要复制的头文件的路径
set(HEADER_DIR include)
# 设置要复制到的目标文件夹
set(HEADER_DEST_DIR ${CMAKE_SOURCE_DIR}/outputs/include)
# 创建目标文件夹
file(MAKE_DIRECTORY ${HEADER_DEST_DIR})
# 复制头文件
file(GLOB_RECURSE HEADER_FILES ${HEADER_DIR}/*.h)
file(COPY ${HEADER_FILES} DESTINATION ${HEADER_DEST_DIR})
#####################复制hv库头文件###################
# 设置要复制的头文件的路径
set(HEADER_HV_DIR libhv_static/include)
# 设置要复制到的目标文件夹
set(HEADER_DEST_HV_DIR ${CMAKE_SOURCE_DIR}/outputs/include_hv)
# 创建目标文件夹
file(MAKE_DIRECTORY ${HEADER_DEST_HV_DIR})
# 复制头文件
file(GLOB_RECURSE HEADER_FILES ${HEADER_HV_DIR}/*.h)
file(COPY ${HEADER_FILES} DESTINATION ${HEADER_DEST_HV_DIR})