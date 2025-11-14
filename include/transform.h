#pragma once
#include <box2d/box2d.h>
#include <glm/glm.hpp>

enum class GetModelFlags { DEFAULT, IGNORE_Z };

struct Transform {
  b2BodyId body_id;
  glm::vec3 position = glm::vec3(0.0f);
  int layer = 0;
  glm::vec2 scale = glm::vec2(1.0f);
  float rotation = 0.0f;
  bool enable_physics = false;

  Transform(b2WorldId world);
  glm::mat4 GetModel(GetModelFlags flags = GetModelFlags::DEFAULT) const;
};
