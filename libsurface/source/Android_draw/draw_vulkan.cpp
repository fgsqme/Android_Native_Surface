//
// Created by fgsqme on 2024/3/23.
//

#include "draw.h"
//#include <font/Font.h>

#include "imgui.h"
#include "vulkan_android.h"
#include "imgui_internal.h"
#include "imgui_impl_android.h"
#include <cstdio>
#include <android/native_window.h>


bool DrawVulkan::initDraw(bool log, int flags) {
    screen_config();
    orientation = displayInfo.orientation;
    return initDraw(displayInfo.width, displayInfo.height, flags, log);
}

bool DrawVulkan::initDraw(uint32_t _screen_x, uint32_t _screen_y, int flags, bool log) {
    width = _screen_x;
    height = _screen_y;
    if (g_Initialized) {
        return true;
    }
    int f = 0;
    // 解决安卓14触摸失效问题（特定情况下设置标志位）
    if (get_android_api_level() >= 33) {
        f |= 0x2000;
    }
    if (flags > 0) {
        f |= flags;
    }
    native_window = externFunction.createNativeWindow("Ssage",
                                                      _screen_x, _screen_y, 1, f, false);
    ANativeWindow_acquire(native_window);
    if (InitVulkan() != 1) {
        ANativeWindow_release(native_window);
        return false;
    }

    // 初始化Vulkan
    SetupVulkan({
                        VK_KHR_SURFACE_EXTENSION_NAME,
                        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
                });

    VkSurfaceKHR surface;
    VkAndroidSurfaceCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .window = native_window
    };
    vkCreateAndroidSurfaceKHR(g_Instance, &createInfo, g_Allocator, &surface);

    // 创建窗口
    ImGui_ImplVulkanH_Window *wd = &g_MainWindowData;
    SetupVulkanWindow(wd, surface, width, height);

    // 初始化Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // 设置渲染器后端
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = g_Instance;
    init_info.PhysicalDevice = g_PhysicalDevice;
    init_info.Device = g_Device;
    init_info.QueueFamily = g_QueueFamily;
    init_info.Queue = g_Queue;
    init_info.PipelineCache = g_PipelineCache;
    init_info.DescriptorPool = g_DescriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = g_MinImageCount;
    init_info.ImageCount = wd->ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = g_Allocator;
    init_info.RenderPass = wd->RenderPass;
    ImGui_ImplVulkan_Init(&init_info);
    ImGuiIO &io = ImGui::GetIO();

    // 初始化Dear ImGui上下文
    io.IniFilename = NULL;
    ImGui_ImplAndroid_Init(native_window);
    ImFontConfig font_cfg;
    font_cfg.SizePixels = 22.0f;
    io.Fonts->AddFontDefault(&font_cfg);
    ImGui::GetStyle().ScaleAllSizes(3.0f);
    g_Initialized = true;
    return true;
}


void DrawVulkan::setDisableRecordState(bool b) {
    if (disableRecord != b) {
        if (b) {
            shutdown();
            initDraw(false, 0x40);
        } else {
            shutdown();
            initDraw();
        }
        disableRecord = b;
    }
}

void DrawVulkan::screen_config() {
    displayInfo = externFunction.getDisplayInfo();
}

bool DrawVulkan::drawBegin() {
    screen_config(); // 更新屏幕配置信息

    bool flag = false;
    // 检查屏幕方向是否变化
    if (orientation != displayInfo.orientation) {
        orientation = displayInfo.orientation;
        printf("width: %d height: %d height: %d\n", displayInfo.width, displayInfo.height, displayInfo.orientation);
        shutdown(); // 关闭Vulkan绘制
        initDraw(); // 重新初始化Vulkan绘制
        flag = true; // 标记屏幕方向变化
    }

    ImGui_ImplVulkan_NewFrame(); // 开始新的Vulkan帧

    if (g_SwapChainRebuild) {
        ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount); // 设置最小图像数量
        ImGui_ImplVulkanH_CreateOrResizeWindow(
                g_Instance,
                g_PhysicalDevice,
                g_Device,
                &g_MainWindowData,
                g_QueueFamily,
                g_Allocator,
                width,
                height,
                g_MinImageCount
        ); // 创建或调整窗口
        g_MainWindowData.FrameIndex = 0; // 重置帧索引
        g_SwapChainRebuild = false; // 标记交换链不需要重建
    }
    // 开始新的Android帧
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();
    return flag; // 返回屏幕方向是否变化的标志
}


void DrawVulkan::drawEnd() {
    ImGui::Render();
    ImDrawData *draw_data = ImGui::GetDrawData();
    FrameRender(&g_MainWindowData, draw_data);
    FramePresent(&g_MainWindowData);
}


void DrawVulkan::shutdown() {
    if (!g_Initialized) {
        return;
    }

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();
    CleanupVulkanWindow();
    CleanupVulkan();
    ANativeWindow_release(native_window);
    g_Initialized = false;
}
