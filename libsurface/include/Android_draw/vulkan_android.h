#ifndef NATIVESURFACE_VULKAN_ANDROID_H
#define NATIVESURFACE_VULKAN_ANDROID_H

#include "imgui.h"
#include "vulkan_android.h"
#include "vulkan_wrapper.h"
#include "imgui_impl_android.h"
#include "imgui_impl_vulkan.h"

#include <vulkan/vulkan.h>
#include <android/native_window.h>
#include <vector>

// Data
extern VkAllocationCallbacks *g_Allocator; // Vulkan分配回调
extern VkInstance g_Instance; // Vulkan实例
extern VkPhysicalDevice g_PhysicalDevice; // 物理设备
extern VkDevice g_Device; // 逻辑设备
extern uint32_t g_QueueFamily; // 队列家族索引
extern VkQueue g_Queue; // 队列
extern VkDebugReportCallbackEXT g_DebugReport; // 调试报告回调
extern VkPipelineCache g_PipelineCache; // 管线缓存
extern VkDescriptorPool g_DescriptorPool; // 描述符池

extern ImGui_ImplVulkanH_Window g_MainWindowData; // 主窗口数据
extern int g_MinImageCount; // 最小图像数量
extern bool g_SwapChainRebuild; // 是否重建交换链

// 初始化Vulkan
void SetupVulkan(std::vector<const char *> instance_extensions);

// 初始化Vulkan窗口
void SetupVulkanWindow(ImGui_ImplVulkanH_Window *wd, VkSurfaceKHR surface, int width, int height);

// 渲染帧
void FrameRender(ImGui_ImplVulkanH_Window *wd, ImDrawData *draw_data);

// 提交帧
void FramePresent(ImGui_ImplVulkanH_Window *wd);

// 清理Vulkan窗口相关资源
void CleanupVulkanWindow();

// 清理Vulkan资源
void CleanupVulkan();

#endif //NATIVESURFACE_VULKAN_ANDROID_H
