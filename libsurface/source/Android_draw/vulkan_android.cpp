//
// Created by fgsqme on 2024/3/23.
//


#include "vulkan_android.h"

// Data
VkAllocationCallbacks *g_Allocator = nullptr;
VkInstance g_Instance = VK_NULL_HANDLE;
VkPhysicalDevice g_PhysicalDevice = VK_NULL_HANDLE;
VkDevice g_Device = VK_NULL_HANDLE;
uint32_t g_QueueFamily = (uint32_t) -1;
VkQueue g_Queue = VK_NULL_HANDLE;
VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
VkPipelineCache g_PipelineCache = VK_NULL_HANDLE;
VkDescriptorPool g_DescriptorPool = VK_NULL_HANDLE;

ImGui_ImplVulkanH_Window g_MainWindowData;
int g_MinImageCount = 2;
bool g_SwapChainRebuild = false;


void check_vk_result(VkResult err) {
    /*  if (err == 0)
          return;
      printf("[vulkan] Error: VkResult = %d\n", err);
      if (err < 0)
          abort();*/
}


VkPhysicalDevice SetupVulkan_SelectPhysicalDevice() {
    uint32_t gpu_count; // GPU数量
    VkResult err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, nullptr); // 获取物理设备列表
    check_vk_result(err); // 检查Vulkan函数调用结果
    IM_ASSERT(gpu_count > 0); // 断言GPU数量大于0

    ImVector<VkPhysicalDevice> gpus; // 存储物理设备的向量
    gpus.resize(gpu_count); // 调整向量大小以容纳物理设备数量
    err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, gpus.Data); // 获取物理设备列表
    check_vk_result(err); // 检查Vulkan函数调用结果

    // 如果报告了多于一个GPU的数量，则查找独立GPU（如果存在），否则使用第一个可用的GPU。这涵盖了
    // 大多数常见情况（多GPU/集成+独立图形）。处理更复杂的设置（多个
    // 专用GPU）不在本示例的范围内。
    for (VkPhysicalDevice &device: gpus) { // 遍历物理设备列表
        VkPhysicalDeviceProperties properties; // 物理设备属性
        vkGetPhysicalDeviceProperties(device, &properties); // 获取物理设备属性
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) // 如果是独立GPU
            return device; // 返回该设备
    }
    if (gpu_count > 0) // 如果有GPU可用
        return gpus[0]; // 返回第一个GPU
    return VK_NULL_HANDLE; // 返回空句柄
}

void SetupVulkan(std::vector<const char *> instance_extensions) {
    VkResult err;
    // 创建 Vulkan 实例
    {
        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        // 设置 Vulkan 实例
        create_info.enabledExtensionCount = (uint32_t) instance_extensions.size();
        create_info.ppEnabledExtensionNames = instance_extensions.data();

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &appInfo;
        // 创建不带任何调试特性的 Vulkan 实例
        err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
        check_vk_result(err);

        // 设置调试报告回调
    }

    // 选择物理设备（GPU）
    g_PhysicalDevice = SetupVulkan_SelectPhysicalDevice();

    // 选择图形队列家族
    {
        uint32_t count;
        vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, nullptr);

        VkQueueFamilyProperties *queues = (VkQueueFamilyProperties *) malloc(sizeof(VkQueueFamilyProperties) * count);
        vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, queues);
        for (uint32_t i = 0; i < count; i++)
            if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                g_QueueFamily = i;
                break;
            }
        free(queues);

        IM_ASSERT(g_QueueFamily != (uint32_t) -1);
    }

    // 创建逻辑设备（带有1个队列）
    {
        ImVector<const char *> device_extensions;
        device_extensions.push_back("VK_KHR_swapchain");

        // 枚举物理设备扩展
        uint32_t properties_count;
        ImVector<VkExtensionProperties> properties;
        vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, nullptr, &properties_count, nullptr);
        properties.resize(properties_count);
        vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, nullptr, &properties_count, properties.Data);

        const float queue_priority[] = {1.0f};
        VkDeviceQueueCreateInfo queue_info[1] = {};
        queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[0].queueFamilyIndex = g_QueueFamily;
        queue_info[0].queueCount = 1;
        queue_info[0].pQueuePriorities = queue_priority;
        VkDeviceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
        create_info.pQueueCreateInfos = queue_info;
        create_info.enabledExtensionCount = (uint32_t) device_extensions.Size;
        create_info.ppEnabledExtensionNames = device_extensions.Data;
        err = vkCreateDevice(g_PhysicalDevice, &create_info, g_Allocator, &g_Device);
        check_vk_result(err);
        vkGetDeviceQueue(g_Device, g_QueueFamily, 0, &g_Queue);
    }

    // 创建描述符池
    {
        VkDescriptorPoolSize pool_sizes[] =
                {
                        {VK_DESCRIPTOR_TYPE_SAMPLER,                1000},
                        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000},
                        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000},
                        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000},
                        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000},
                        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000},
                        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000},
                        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000}
                };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = (uint32_t) IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        err = vkCreateDescriptorPool(g_Device, &pool_info, g_Allocator, &g_DescriptorPool);
        check_vk_result(err);
    }
}


void SetupVulkanWindow(ImGui_ImplVulkanH_Window *wd, VkSurfaceKHR surface, int width, int height) {
    wd->Surface = surface; // 设置窗口表面

    // 检查WSI支持
    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, g_QueueFamily, wd->Surface, &res);
    if (res != VK_TRUE) { // 如果不支持WSI
        fprintf(stderr, "Error no WSI support on physical device 0\n");
        exit(-1);
    }

    // 选择表面格式
    const VkFormat requestSurfaceImageFormat[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(g_PhysicalDevice, wd->Surface, requestSurfaceImageFormat,
                                                              (size_t) IM_ARRAYSIZE(requestSurfaceImageFormat),
                                                              requestSurfaceColorSpace);

    // 选择呈现模式
    VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(g_PhysicalDevice, wd->Surface, &present_modes[0],
                                                          IM_ARRAYSIZE(present_modes));
    //printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);
    // 创建交换链、渲染通道、帧缓冲区等
    IM_ASSERT(g_MinImageCount >= 2);
    ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, wd, g_QueueFamily, g_Allocator,
                                           width, height, g_MinImageCount);
}


void CleanupVulkan() {
    // 销毁描述符池
    vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);
    // 销毁逻辑设备
    vkDestroyDevice(g_Device, g_Allocator);
    // 销毁 Vulkan 实例
    vkDestroyInstance(g_Instance, g_Allocator);
}

void CleanupVulkanWindow() {
    // 销毁窗口相关资源
    ImGui_ImplVulkanH_DestroyWindow(g_Instance, g_Device, &g_MainWindowData, g_Allocator);
}


void FrameRender(ImGui_ImplVulkanH_Window *wd, ImDrawData *draw_data) {
    VkResult err;

    VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    // 获取下一帧图像
    err = vkAcquireNextImageKHR(g_Device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE,
                                &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
        g_SwapChainRebuild = true;
        return;
    }
    check_vk_result(err);

    ImGui_ImplVulkanH_Frame *fd = &wd->Frames[wd->FrameIndex];
    {
        // 等待之前提交的命令完成
        err = vkWaitForFences(g_Device, 1, &fd->Fence, VK_TRUE,
                              UINT64_MAX);    // 等待，而不是周期性检查
        check_vk_result(err);

        // 重置fence
        err = vkResetFences(g_Device, 1, &fd->Fence);
        check_vk_result(err);
    }
    {
        // 重置命令池
        err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        check_vk_result(err);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        // 开始渲染通道
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // 将 dear imgui 绘制数据记录到命令缓冲区中
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // 结束渲染通道
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        check_vk_result(err);
        // 提交命令缓冲区
        err = vkQueueSubmit(g_Queue, 1, &info, fd->Fence);
        check_vk_result(err);
    }
}

void FramePresent(ImGui_ImplVulkanH_Window *wd) {
    if (g_SwapChainRebuild)
        return;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    VkResult err = vkQueuePresentKHR(g_Queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
        g_SwapChainRebuild = true;
        return;
    }
    check_vk_result(err);
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount; // 现在可以使用下一组信号量
}
