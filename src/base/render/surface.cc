#define GLFW_INCLUDE_VULKAN
#include "ion/render.h"
#include "ion/render/api.h"
#include <GLFW/glfw3.h>

void ion::render::api::CreateSurface() {
  if (glfwCreateWindowSurface(internal::instance, ion::render::GetWindow(),
                              nullptr, &internal::surface) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create surface");
  }
}
