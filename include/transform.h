#pragma once
#include "texture.h"
#include <box2d/box2d.h>
#include <glm/glm.hpp>

enum class GetModelFlags { DEFAULT, IGNORE_LAYER };

class Transform {
private:
  b2BodyId body_id;
  Texture texture;

public:
  glm::vec2 position = glm::vec2(0.0f);
  int layer = 0;
  glm::vec2 scale = glm::vec2(1.0f);
  float rotation = 0.0f;
  bool enable_physics = false;
  bool lock_camera = false;

  Transform(b2WorldId world);
  Texture &GetTexture();
  void UpdatePhysics();
  void RenderInspector(int id, b2WorldId world);
  glm::mat4 GetModel(GetModelFlags flags = GetModelFlags::DEFAULT);
};
