#include "ion/render/api.h"

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
std::vector<VkSemaphore> image_available_semaphore{}, render_finished_semaphore{};
std::vector<VkFence> in_flight_fence{};

VkDescriptorPool descriptor_pool{};
} // namespace ion::render::api::internal

void ion::render::api::Quit() {
  vkDeviceWaitIdle(internal::device);
  for (auto semaphore : internal::render_finished_semaphore) {
    vkDestroySemaphore(internal::device, semaphore, nullptr);
  }
  for (auto semaphore : internal::image_available_semaphore) {
    vkDestroySemaphore(internal::device, semaphore, nullptr);
  }
  for (auto fence : internal::in_flight_fence) {
    vkDestroyFence(internal::device, fence, nullptr);
  }
  for (auto framebuffer : internal::swapchain_framebuffers) {
    vkDestroyFramebuffer(internal::device, framebuffer, nullptr);
  }
  vkDestroyPipeline(internal::device, internal::graphics_pipeline, nullptr);
  vkDestroyPipelineLayout(internal::device, internal::pipeline_layout, nullptr);
  vkDestroyRenderPass(internal::device, internal::render_pass, nullptr);
  for (auto view : internal::swapchain_image_views) {
    vkDestroyImageView(internal::device, view, nullptr);
  }
  vkDestroySwapchainKHR(internal::device, internal::swapchain, nullptr);
  vkDestroyDevice(internal::device, nullptr);
  vkDestroySurfaceKHR(internal::instance, internal::surface, nullptr);
  vkDestroyInstance(internal::instance, nullptr);
}
