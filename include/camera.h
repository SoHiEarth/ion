#pragma once
#include <glm/glm.hpp>

enum class ProjectionMode : std::uint8_t { PERSPECTIVE = 0, ORTHOGRAPHIC = 1 };
struct Camera {
  glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f);
  ProjectionMode mode = ProjectionMode::PERSPECTIVE;
  float fov = glm::radians(45.0f);
};
