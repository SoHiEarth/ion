#include "ion/render/api.h"
#include <GLFW/glfw3.h>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

bool CheckValidationLayerSupport() {
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  auto available_layers = std::vector<VkLayerProperties>{layer_count};
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  for (auto name : ion::render::api::internal::validation_layers) {
    bool layer_found = false;
    for (const auto &properties : available_layers) {
      if (strcmp(name, properties.layerName) == 0) {
        layer_found = true;
        break;
      }
    }
    if (!layer_found) {
      return false;
    }
    return false;
  }
  return true;
}

std::vector<const char *> GetRequiredExtensions() {
  uint32_t glfw_extension_count = 0;
  auto glfw_extensions =
      glfwGetRequiredInstanceExtensions(&glfw_extension_count);
  auto extensions = std::vector<const char *>{
      glfw_extensions, glfw_extensions + glfw_extension_count};

  if (ion::render::api::internal::enable_validation_layers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  return extensions;
}

void ion::render::api::CreateInstance() {
  if (internal::enable_validation_layers && !CheckValidationLayerSupport()) {
    throw std::runtime_error("Requested validation layers were not available.");
  }
  auto app_info = VkApplicationInfo{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "ion";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "ion";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  auto create_info = VkInstanceCreateInfo{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;

  auto extensions = GetRequiredExtensions();
  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();

  if (internal::enable_validation_layers) {
    create_info.enabledLayerCount = static_cast<uint32_t>(
        ion::render::api::internal::validation_layers.size());
    create_info.ppEnabledLayerNames =
        ion::render::api::internal::validation_layers.data();
  } else {
    create_info.enabledLayerCount = 0;
  }

  if (vkCreateInstance(&create_info, nullptr, &internal::instance) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create instance");
  }
}
