#include "ion/render/api.h"
#include <stdexcept>
void ion::render::api::CreateImageViews() {
  internal::swapchain_image_views.resize(internal::swapchain_images.size());
  for (size_t i = 0; i < internal::swapchain_images.size(); i++) {
    auto info = VkImageViewCreateInfo{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.image = internal::swapchain_images.at(i);
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = internal::swapchain_image_format;
    info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    if (vkCreateImageView(internal::device, &info, nullptr,
                          &internal::swapchain_image_views.at(i)) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to create image view");
    }
  }
}
