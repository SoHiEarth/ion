#include "ion/render/api.h"
#include "ion/development/gui.h"

namespace ion::render::api::internal {
VkInstance instance{};
VkDebugUtilsMessengerEXT messenger{};
VkPhysicalDevice physical_device = VK_NULL_HANDLE;
VkDevice device{};
VkQueue graphics_queue{}, present_queue{};
VkSurfaceKHR surface{};
VkSwapchainKHR swapchain{};
std::vector<VkImageView> swapchain_image_views{};
std::vector<VkImage> swapchain_images{};
VkFormat swapchain_image_format{};
VkExtent2D swapchain_extent{};
VkRenderPass render_pass{};
VkPipelineLayout pipeline_layout{};
VkPipeline graphics_pipeline{};
std::vector<VkFramebuffer> swapchain_framebuffers{};
VkCommandPool command_pool{};
std::vector<VkCommandBuffer> command_buffer{};
std::vector<VkSemaphore> image_available_semaphore{},
    render_finished_semaphore{};
std::vector<VkFence> in_flight_fence{};
VkBuffer vertex_buffer{};
VkDeviceMemory vertex_buffer_memory{};
VkDescriptorPool descriptor_pool{};
VkAllocationCallbacks *allocator = nullptr;
} // namespace ion::render::api::internal

void ion::render::api::Quit() {
  vkDeviceWaitIdle(internal::device);
  for (auto framebuffer : internal::swapchain_framebuffers) {
    vkDestroyFramebuffer(internal::device, framebuffer, internal::allocator);
  }
  for (auto view : internal::swapchain_image_views) {
    vkDestroyImageView(internal::device, view, internal::allocator);
  }
  vkDestroySwapchainKHR(internal::device, internal::swapchain,
                        internal::allocator);
  vkDestroyPipeline(internal::device, internal::graphics_pipeline,
                    internal::allocator);
  vkDestroyPipelineLayout(internal::device, internal::pipeline_layout,
                          internal::allocator);
  vkDestroyRenderPass(internal::device, internal::render_pass,
                      internal::allocator);
  vkDestroyBuffer(internal::device, internal::vertex_buffer,
                  internal::allocator);
  vkFreeMemory(internal::device, internal::vertex_buffer_memory,
               internal::allocator);
  for (size_t i = 0; i < internal::max_frames_in_flight; i++) {
    vkDestroySemaphore(internal::device, internal::render_finished_semaphore[i],
                       internal::allocator);
    vkDestroySemaphore(internal::device, internal::image_available_semaphore[i],
                       internal::allocator);
    vkDestroyFence(internal::device, internal::in_flight_fence[i],
                   internal::allocator);
  }
  vkDestroyCommandPool(internal::device, internal::command_pool,
                       internal::allocator);
  vkDestroyDescriptorPool(internal::device, internal::descriptor_pool,
                          internal::allocator);
  vkDestroyDevice(internal::device, internal::allocator);
  if (internal::enable_validation_layers) {
    DestroyMessenger();
  }
  vkDestroySurfaceKHR(internal::instance, internal::surface,
                      internal::allocator);
  vkDestroyInstance(internal::instance, internal::allocator);
}
