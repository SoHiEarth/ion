#include "ion/render.h"
#include "ion/render/api.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

VkSurfaceFormatKHR
ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &available_formats) {
  for (const auto &available_format : available_formats) {
    if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return available_format;
    }
  }

  return available_formats.front();
}

VkPresentModeKHR
ChoosePresentMode(const std::vector<VkPresentModeKHR> &available_modes) {
  for (const auto &available_mode : available_modes) {
    if (available_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return available_mode;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(ion::render::GetWindow(), &width, &height);
    VkExtent2D actual_extent = {static_cast<uint32_t>(width),
                                static_cast<uint32_t>(height)};
    actual_extent.width =
        std::clamp(actual_extent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actual_extent.height =
        std::clamp(actual_extent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);
    return actual_extent;
  }
}

void ion::render::api::CreateSwapchain() {
  auto support = internal::QuerySwapchainSupport(internal::physical_device);
  auto surface_format = ChooseSurfaceFormat(support.formats);
  auto present_mode = ChoosePresentMode(support.present_modes);
  auto extent = ChooseExtent(support.capabilities);
  uint32_t image_count = support.capabilities.minImageCount + 1;
  if (support.capabilities.maxImageCount > 0 &&
      image_count > support.capabilities.maxImageCount) {
    image_count = support.capabilities.maxImageCount;
  }

  auto info = VkSwapchainCreateInfoKHR{};
  info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  info.surface = internal::surface;
  info.minImageCount = image_count;
  info.imageFormat = surface_format.format;
  info.imageColorSpace = surface_format.colorSpace;
  info.imageExtent = extent;
  info.imageArrayLayers = 1;
  info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  auto indices = internal::FindQueueFamilies(internal::physical_device);
  uint32_t queue_family_indices[] = {indices.graphics_family.value(),
                                     indices.present_family.value()};
  if (indices.graphics_family != indices.present_family) {
    info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    info.queueFamilyIndexCount = 2;
    info.pQueueFamilyIndices = queue_family_indices;
  } else {
    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.queueFamilyIndexCount = 0;
    info.pQueueFamilyIndices = nullptr;
  }
  info.preTransform = support.capabilities.currentTransform;
  info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  info.presentMode = present_mode;
  info.clipped = VK_TRUE;
  info.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(internal::device, &info, nullptr,
                           &internal::swapchain) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create swapchain");
  }

  vkGetSwapchainImagesKHR(internal::device, internal::swapchain, &image_count,
                          nullptr);
  internal::swapchain_images.resize(image_count);
  vkGetSwapchainImagesKHR(internal::device, internal::swapchain, &image_count,
                          internal::swapchain_images.data());
  internal::swapchain_image_format = surface_format.format;
  internal::swapchain_extent = extent;
}
