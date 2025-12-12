#pragma once
#include <cstdio>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace ion::render::api {

struct QueueFamilyIndices {
  std::optional<uint32_t> graphics_family, present_family;
};

struct SwapchainSupportInfo {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

namespace internal {
#ifdef NDEBUG
static const bool enable_validation_layers = false;
#else
static const bool enable_validation_layers = true;
#endif

static const std::vector<const char *>
    validation_layers = {"VK_LAYER_KHRONOS_validation"},
    device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *data, void *user_data) {
  printf("Validation Layer: %s\n", data->pMessage);
  return false;
}

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
SwapchainSupportInfo QuerySwapchainSupport(VkPhysicalDevice device);

extern VkInstance instance;
extern VkDebugUtilsMessengerEXT messenger;
extern VkPhysicalDevice physical_device;
extern VkDevice device;
extern VkQueue graphics_queue, present_queue;
extern VkSurfaceKHR surface;
extern VkSwapchainKHR swapchain;
extern std::vector<VkImageView> swapchain_image_views;
extern std::vector<VkImage> swapchain_images;
extern VkFormat swapchain_image_format;
extern VkExtent2D swapchain_extent;
extern VkRenderPass render_pass;
extern VkPipelineLayout pipeline_layout;
extern VkPipeline graphics_pipeline;
extern std::vector<VkFramebuffer> swapchain_framebuffers;
extern VkCommandPool command_pool;
extern VkCommandBuffer command_buffer;
extern VkSemaphore image_available_semaphore, render_finished_semaphore;
extern VkFence in_flight_fence;
} // namespace internal

void CreateInstance();
void CreateMessenger();
void CreateSurface();
void CreateDevice();
void CreateSwapchain();
void CreateImageViews();
void CreatePipeline();
void CreateRenderPass();
void CreateFramebuffers();
void CreateSyncObjects();
void CreateCommandPool();
void CreateCommandBuffer();

void RecordCommandBuffer(VkCommandBuffer, uint32_t);
void Render();

void DestroyMessenger();
void Quit();
} // namespace ion::render::api
