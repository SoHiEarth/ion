#pragma once
#include <glm/glm.hpp>
#include <box2d/box2d.h>
#include <memory>

struct Texture;
struct Shader;
struct GPUData;

enum class ProjectionMode : std::uint8_t { PERSPECTIVE = 0, ORTHOGRAPHIC = 1 };

struct Transform {
  glm::vec2 position = glm::vec2(0.0f);
  int layer = 0;
  glm::vec2 scale = glm::vec2(1.0f);
  float rotation = 0.0f;
};

struct PhysicsBody {
  b2BodyId body_id;
  bool enabled = false;
};

struct Renderable {
  std::shared_ptr<Texture> texture;
  std::shared_ptr<Shader> shader;
  std::shared_ptr<GPUData> data;
};

struct Camera {
  glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f);
  ProjectionMode mode = ProjectionMode::PERSPECTIVE;
};