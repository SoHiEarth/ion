#include "ion/render/api.h"

namespace ion::render::api::internal {
VkInstance instance{};
VkDebugUtilsMessengerEXT messenger{};
VkPhysicalDevice physical_device = VK_NULL_HANDLE;
VkDevice device{};
VkQueue graphics_queue{}, present_queue{};
VkSurfaceKHR surface{};
VkSwapchainKHR swapchain{};
std::vector<VkImageView> swapchain_image_views;
std::vector<VkImage> swapchain_images;
VkFormat swapchain_image_format;
VkExtent2D swapchain_extent;
} // namespace ion::render::api::internal

void ion::render::api::Quit() {
  for (auto view : internal::swapchain_image_views) {
    vkDestroyImageView(internal::device, view, nullptr);
  }
  vkDestroySwapchainKHR(internal::device, internal::swapchain, nullptr);
  vkDestroyDevice(internal::device, nullptr);
  vkDestroySurfaceKHR(internal::instance, internal::surface, nullptr);
  vkDestroyInstance(internal::instance, nullptr);
}
