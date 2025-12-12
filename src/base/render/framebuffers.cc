#include "ion/render/api.h"
#include <stdexcept>
void ion::render::api::CreateFramebuffers() {
  internal::swapchain_framebuffers.resize(
      internal::swapchain_image_views.size());
  for (size_t i = 0; i < internal::swapchain_image_views.size(); i++) {
    VkImageView attachments[] = {internal::swapchain_image_views.at(i)};
    auto info = VkFramebufferCreateInfo{};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = internal::render_pass;
    info.attachmentCount = 1;
    info.pAttachments = attachments;
    info.width = internal::swapchain_extent.width;
    info.height = internal::swapchain_extent.height;
    info.layers = 1;
    if (vkCreateFramebuffer(internal::device, &info, nullptr,
                            &internal::swapchain_framebuffers.at(i)) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to create framebuffer");
    }
  }
}
