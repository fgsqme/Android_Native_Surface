//
// Created by 泓清 on 2022/8/26.
//

#include "draw.h"
//#include "vulkan_engine.h"
//#include <font/Font.h>

#include "imgui.h"
#include "vulkan_android.h"
#include "imgui_internal.h"
#include "imgui_impl_android.h"
#include <cstdio>
#include <android/native_window.h>
#include <vector>

//vulkan_engine *vulkanGraphics;

bool DrawVulkan::initDraw(int flags, bool log) {
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
    // 解决安卓14触摸失效问题
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

    SetupVulkan({
                        VK_KHR_SURFACE_EXTENSION_NAME,
                        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
                });
    VkSurfaceKHR surface;
    VkAndroidSurfaceCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .window = native_window};
    vkCreateAndroidSurfaceKHR(g_Instance, &createInfo, g_Allocator,
                              &surface);
    // Create Framebuffers
    ImGui_ImplVulkanH_Window *wd = &g_MainWindowData;
    SetupVulkanWindow(wd, surface, width, height);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
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
//    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info);
    ImGuiIO &io = ImGui::GetIO();

    // Setup Dear ImGui context
    io.IniFilename = NULL;
    ImGui::StyleColorsDark();
    ImGui_ImplAndroid_Init(native_window);
    ImFontConfig font_cfg;
    font_cfg.SizePixels = 22.0f;
    io.Fonts->AddFontDefault(&font_cfg);
//    ImFontConfig font_cfg;
//    io.Fonts->AddFontFromMemoryTTF((void*)OPPOSans_H, OPPOSans_H_size, 22.0f, &font_cfg);
    ImGui::GetStyle().ScaleAllSizes(3.0f);
    g_Initialized = true;
    return true;
}

void DrawVulkan::setDisableRecordState(bool b) {
    if (disableRecord != b) {
        shutdown();
        initDraw();
    }
}

void DrawVulkan::screen_config() {
    displayInfo = externFunction.getDisplayInfo();
}


bool DrawVulkan::drawBegin() {
    screen_config();



    bool flag = false;
    if (orientation != displayInfo.orientation) {
        orientation = displayInfo.orientation;
        printf("width: %d height: %d height: %d\n", displayInfo.width, displayInfo.height, displayInfo.orientation);
        shutdown();
        initDraw();
        flag = true;
    }
    ImGui_ImplVulkan_NewFrame();

    if (g_SwapChainRebuild) {
        ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
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
        );
        g_MainWindowData.FrameIndex = 0;
        g_SwapChainRebuild = false;
    }
    ImGui_ImplAndroid_NewFrame();
    // Setup time step
    ImGui::NewFrame();
    return flag;
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
