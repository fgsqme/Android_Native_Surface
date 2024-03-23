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
extern VkAllocationCallbacks *g_Allocator;
extern VkInstance g_Instance;
extern VkPhysicalDevice g_PhysicalDevice;
extern VkDevice g_Device;
extern uint32_t g_QueueFamily;
extern VkQueue g_Queue;
extern VkDebugReportCallbackEXT g_DebugReport;
extern VkPipelineCache g_PipelineCache;
extern VkDescriptorPool g_DescriptorPool;

extern ImGui_ImplVulkanH_Window g_MainWindowData;
extern int g_MinImageCount;
extern bool g_SwapChainRebuild;


void SetupVulkan(std::vector<const char *> instance_extensions);

void SetupVulkanWindow(ImGui_ImplVulkanH_Window *wd, VkSurfaceKHR surface, int width, int height);

void FrameRender(ImGui_ImplVulkanH_Window *wd, ImDrawData *draw_data);

void FramePresent(ImGui_ImplVulkanH_Window *wd);

void CleanupVulkanWindow();

void CleanupVulkan();

#endif //NATIVESURFACE_VULKAN_ANDROID_H
