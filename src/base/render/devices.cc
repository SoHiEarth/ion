#include "ion/render/api.h"
#include <cstdint>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

ion::render::api::QueueFamilyIndices
ion::render::api::internal::FindQueueFamilies(VkPhysicalDevice device) {
  auto indices = QueueFamilyIndices{};
  uint32_t count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
  auto families = std::vector<VkQueueFamilyProperties>{count};
  vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

  for (int i = 0; i < families.size(); i++) {
    if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics_family = i;
    }
    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
    if (present_support) {
      indices.present_family = i;
    }
  }

  return indices;
}

bool DeviceSupportsExtensions(VkPhysicalDevice device) {
  uint32_t extension_count;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                       nullptr);
  auto available_extensions =
      std::vector<VkExtensionProperties>{extension_count};
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                       available_extensions.data());
  std::set<std::string> required_extensions(
      ion::render::api::internal::device_extensions.begin(),
      ion::render::api::internal::device_extensions.end());
  for (const auto &extension : available_extensions) {
    required_extensions.erase(extension.extensionName);
  }

  return required_extensions.empty();
}

ion::render::api::SwapchainSupportInfo
ion::render::api::internal::QuerySwapchainSupport(VkPhysicalDevice device) {
  auto support_info = SwapchainSupportInfo{};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &support_info.capabilities);

  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
  if (format_count != 0) {
    support_info.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count,
                                         support_info.formats.data());
  }

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                            &present_mode_count, nullptr);
  if (present_mode_count != 0) {
    support_info.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &present_mode_count,
        support_info.present_modes.data());
  }

  return support_info;
}

int RateDevice(VkPhysicalDevice device) {
  auto properties = VkPhysicalDeviceProperties{};
  vkGetPhysicalDeviceProperties(device, &properties);
  auto features = VkPhysicalDeviceFeatures{};
  vkGetPhysicalDeviceFeatures(device, &features);
  auto indices = ion::render::api::internal::FindQueueFamilies(device);

  int score = 0;
  if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    score += 1000;
  }
  score += properties.limits.maxImageDimension2D;
  if (!features.geometryShader) {
    return 0;
  }

  if (!indices.graphics_family.has_value() ||
      !indices.present_family.has_value()) {
    return 0;
  }

  bool swapchain_supported = false;
  if (DeviceSupportsExtensions(device)) {
    auto support_info =
        ion::render::api::internal::QuerySwapchainSupport(device);
    swapchain_supported =
        !support_info.formats.empty() && !support_info.present_modes.empty();
  }
  if (!swapchain_supported) {
    return 0;
  }

  return score;
}

void ion::render::api::CreateDevice() {
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(internal::instance, &device_count, nullptr);
  if (device_count == 0) {
    throw std::runtime_error("Failed to find GPUs");
  }
  auto devices = std::vector<VkPhysicalDevice>{device_count};
  vkEnumeratePhysicalDevices(internal::instance, &device_count, devices.data());

  auto rated_devices = std::multimap<int, VkPhysicalDevice>{};
  for (const auto &device : devices) {
    rated_devices.insert({RateDevice(device), device});
  }
  if (rated_devices.rbegin()->first > 0) {
    internal::physical_device = rated_devices.rbegin()->second;
  } else {
    throw std::runtime_error("Failed to find a suitable GPU");
  }

  auto indices = internal::FindQueueFamilies(internal::physical_device);
  auto queue_create_infos = std::vector<VkDeviceQueueCreateInfo>{};
  auto unique_queue_families = std::set<uint32_t>{
      indices.graphics_family.value(), indices.present_family.value()};
  float priority = 1.0f;
  for (uint32_t family : unique_queue_families) {
    auto queue_info = VkDeviceQueueCreateInfo{};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = family;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priority;
    queue_create_infos.push_back(queue_info);
  }

  auto features = VkPhysicalDeviceFeatures{};
  auto info = VkDeviceCreateInfo{};
  info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
  info.pQueueCreateInfos = queue_create_infos.data();
  info.pEnabledFeatures = &features;

  info.enabledExtensionCount =
      static_cast<uint32_t>(internal::device_extensions.size());
  info.ppEnabledExtensionNames = internal::device_extensions.data();

  if (internal::enable_validation_layers) {
    info.enabledLayerCount =
        static_cast<uint32_t>(internal::validation_layers.size());
    info.ppEnabledLayerNames = internal::validation_layers.data();
  } else {
    info.enabledLayerCount = 0;
  }

  if (vkCreateDevice(internal::physical_device, &info, nullptr,
                     &internal::device) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create device");
  }
  vkGetDeviceQueue(internal::device, indices.graphics_family.value(), 0,
                   &internal::graphics_queue);
  vkGetDeviceQueue(internal::device, indices.present_family.value(), 0,
                   &internal::present_queue);
}
