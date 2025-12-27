#include "ion/data.h"
#include <vulkan/vulkan_core.h>

VkVertexInputBindingDescription Vertex::GetBindingDescription() {
  auto desc = VkVertexInputBindingDescription{};
  desc.binding = 0;
  desc.stride = sizeof(Vertex);
  desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return desc;
}

std::array<VkVertexInputAttributeDescription, 2>
Vertex::GetAttributeDescriptions() {
  std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions{};
  auto &position_attribute = attribute_descriptions.at(0);
  position_attribute.binding = 0;
  position_attribute.location = 0;
  position_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
  position_attribute.offset = offsetof(Vertex, position);
  auto &color_attribute = attribute_descriptions.at(1);
  color_attribute.binding = 0;
  color_attribute.location = 1;
  color_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
  color_attribute.offset = offsetof(Vertex, color);
  return attribute_descriptions;
}
