#pragma once
#include "transform.h"
#include <vector>
class Texture;
class Shader;

struct Sprites {
  std::vector<Transform> transforms;
  std::vector<Texture> textures;
  void New(Transform transform, Texture texture);
  void Draw(Shader &shader, GetModelFlags model_flags);
  void Inspector();
};
