#pragma once
#include "exports.h"
#include <array>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

struct ION_API Vertex {
  glm::vec3 position;
  glm::vec3 color;

  static VkVertexInputBindingDescription GetBindingDescription();
  static std::array<VkVertexInputAttributeDescription, 2>
  GetAttributeDescriptions();
};

struct ION_API Data {
private:
  std::string id;

public:
  const std::vector<Vertex> vertices;
  const std::vector<unsigned int> indices;
  bool element_enabled = false;
  const std::string &GetID() const { return id; }
  Data(std::string_view id_) : id(std::string(id_)) {}
};
