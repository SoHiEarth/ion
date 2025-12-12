#include "ion/render/api.h"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

void ion::render::api::CreateCommandPool() {
  auto indices = internal::FindQueueFamilies(internal::physical_device);
  auto pool_info = VkCommandPoolCreateInfo{};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_info.queueFamilyIndex = indices.graphics_family.value();
  if (vkCreateCommandPool(internal::device, &pool_info, nullptr,
                          &internal::command_pool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create command pool");
  }
}

void ion::render::api::CreateCommandBuffer() {
  auto alloc_info = VkCommandBufferAllocateInfo{};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = internal::command_pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = 1;
  if (vkAllocateCommandBuffers(internal::device, &alloc_info,
                               &internal::command_buffer) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create command buffers");
  }
}

void ion::render::api::CreateSyncObjects() {
  auto semaphore_info = VkSemaphoreCreateInfo{};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  auto fence_info = VkFenceCreateInfo{};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  if (vkCreateSemaphore(internal::device, &semaphore_info, nullptr,
                        &internal::image_available_semaphore) != VK_SUCCESS ||
      vkCreateSemaphore(internal::device, &semaphore_info, nullptr,
                        &internal::render_finished_semaphore) != VK_SUCCESS ||
      vkCreateFence(internal::device, &fence_info, nullptr,
                    &internal::in_flight_fence) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create sync objects");
  }
}

void ion::render::api::RecordCommandBuffer(VkCommandBuffer buffer,
                                           uint32_t index) {
  auto begin_info = VkCommandBufferBeginInfo{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = 0;
  begin_info.pInheritanceInfo = nullptr;
  if (vkBeginCommandBuffer(buffer, &begin_info) != VK_SUCCESS) {
    throw std::runtime_error("Failed to begin recording command buffer");
  }

  auto render_pass_info = VkRenderPassBeginInfo{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = internal::render_pass;
  render_pass_info.framebuffer = internal::swapchain_framebuffers.at(index);
  render_pass_info.renderArea.offset = {0, 0};
  render_pass_info.renderArea.extent = internal::swapchain_extent;
  auto clear_color = VkClearValue{0.0, 0.0, 0.0, 0.0};
  render_pass_info.clearValueCount = 1;
  render_pass_info.pClearValues = &clear_color;
  vkCmdBeginRenderPass(buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    internal::graphics_pipeline);
  auto viewport = VkViewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(internal::swapchain_extent.width);
  viewport.height = static_cast<float>(internal::swapchain_extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(buffer, 0, 1, &viewport);

  auto scissor = VkRect2D{};
  scissor.offset = {0, 0};
  scissor.extent = internal::swapchain_extent;
  vkCmdSetScissor(buffer, 0, 1, &scissor);

  vkCmdDraw(buffer, 3, 1, 0, 0);

  vkCmdEndRenderPass(internal::command_buffer);
  if (vkEndCommandBuffer(internal::command_buffer) != VK_SUCCESS) {
    throw std::runtime_error("Failed to end command buffer");
  }
}

void ion::render::api::Render() {
  vkWaitForFences(internal::device, 1, &internal::in_flight_fence, VK_TRUE,
                  UINT64_MAX);
  vkResetFences(internal::device, 1, &internal::in_flight_fence);

  uint32_t image_index;
  vkAcquireNextImageKHR(internal::device, internal::swapchain, UINT64_MAX,
                        internal::image_available_semaphore, VK_NULL_HANDLE,
                        &image_index);
  vkResetCommandBuffer(internal::command_buffer, 0);
  RecordCommandBuffer(internal::command_buffer, 0);

  auto submit_info = VkSubmitInfo{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore wait_semaphores[] = {internal::image_available_semaphore};
  VkPipelineStageFlags wait_stages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = wait_semaphores;
  submit_info.pWaitDstStageMask = wait_stages;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &internal::command_buffer;

  VkSemaphore signal_semaphores[] = {internal::render_finished_semaphore};
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = signal_semaphores;

  if (vkQueueSubmit(internal::graphics_queue, 1, &submit_info,
                    internal::in_flight_fence) != VK_SUCCESS) {
    throw std::runtime_error("Failed to submit draw command buffer");
  }

  auto present_info = VkPresentInfoKHR{};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = signal_semaphores;
  VkSwapchainKHR swapchains[] = {internal::swapchain};
  present_info.swapchainCount = 1;
  present_info.pSwapchains = swapchains;
  present_info.pImageIndices = &image_index;
  present_info.pResults = nullptr;
  vkQueuePresentKHR(internal::present_queue, &present_info);
}
