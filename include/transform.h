#pragma once
#include <box2d/box2d.h>
#include <glm/glm.hpp>

class Transform {
private:
  b2BodyId body_id;

public:
  glm::vec2 position = glm::vec2(0.0f);
  int layer = 0;
  glm::vec2 scale = glm::vec2(1.0f);
  float rotation = 0.0f;
  bool enable_physics = false;

  Transform(b2WorldId world);
  void UpdatePhysics();
  void RenderInspector(int id);
  glm::mat4 GetModel();
};
