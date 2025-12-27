#pragma once
#include "ion/data.h"
#include <cstdio>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

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
const int max_frames_in_flight = 2;

// TEMP
static std::vector<Vertex> vertices = {
    {{0.0f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

#ifdef NDEBUG
static const bool enable_validation_layers = false;
#else
static const bool enable_validation_layers = true;
#endif
static bool using_fallback_layer = false;

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

extern VkAllocationCallbacks *allocator;
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
extern std::vector<VkCommandBuffer> command_buffer;
extern std::vector<VkSemaphore> image_available_semaphore,
    render_finished_semaphore;
extern std::vector<VkFence> in_flight_fence;
extern VkBuffer vertex_buffer;
extern VkDeviceMemory vertex_buffer_memory;
extern VkDescriptorPool descriptor_pool;
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
void CreateCommandPool();
void CreateVertexBuffer(const std::vector<Vertex> &vertices);
void CreateCommandBuffer();
void CreateSyncObjects();

void RecordCommandBuffer(VkCommandBuffer, uint32_t);
void Render();

void DestroyMessenger();
void Quit();
} // namespace ion::render::api
