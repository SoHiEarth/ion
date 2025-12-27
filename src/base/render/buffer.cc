#include "ion/data.h"
#include "ion/render/api.h"
#include <cstring>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

uint32_t FindMemoryType(uint32_t filter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(
      ion::render::api::internal::physical_device, &memory_properties);
  for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
    if (filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags &
                              properties) == properties) {
      return i;
    }
  }
  throw std::runtime_error("Failed to find suitable memory type");
}

void ion::render::api::CreateVertexBuffer(const std::vector<Vertex> &vertices) {
  VkBufferCreateInfo buffer_info{};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = sizeof(vertices[0]) * vertices.size();
  buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  if (vkCreateBuffer(internal::device, &buffer_info, internal::allocator,
                     &internal::vertex_buffer) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create vertex buffer");
  }

  VkMemoryRequirements memory_requirements{};
  vkGetBufferMemoryRequirements(internal::device, internal::vertex_buffer,
                                &memory_requirements);
  VkMemoryAllocateInfo allocate_info{};
  allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.allocationSize = memory_requirements.size;
  allocate_info.memoryTypeIndex =
      FindMemoryType(memory_requirements.memoryTypeBits,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  if (vkAllocateMemory(internal::device, &allocate_info, internal::allocator,
                       &internal::vertex_buffer_memory) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate buffer memory");
  }

  vkBindBufferMemory(internal::device, internal::vertex_buffer,
                     internal::vertex_buffer_memory, 0);

  void *data;
  vkMapMemory(internal::device, internal::vertex_buffer_memory, 0,
              buffer_info.size, 0, &data);
  memcpy(data, vertices.data(), (size_t)buffer_info.size);
  vkUnmapMemory(internal::device, internal::vertex_buffer_memory);
}
