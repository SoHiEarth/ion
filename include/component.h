#pragma once
#include <glm/glm.hpp>
#include "texture.h"
#include "shader.h"
#include "physics.h"

enum class ProjectionMode : std::uint8_t { PERSPECTIVE = 0, ORTHOGRAPHIC = 1 };

struct Transform {
  glm::vec3 position = glm::vec3(0.0f);
  glm::vec2 scale = glm::vec2(1.0f);
  float rotation = 0.0f;
};

struct PhysicsBody {
  b2BodyId body_id;
  bool enabled = false;
};

struct Renderable {
  Texture texture;
  Shader shader;
};

struct Camera {
  glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f);
  ProjectionMode mode = ProjectionMode::PERSPECTIVE;
};