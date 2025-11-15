#pragma once
#include "texture.h"
#include "transform.h"
#include <vector>
class Texture;
class Shader;
struct RenderSystem;

struct Sprites {
private:
  RenderSystem &render;

public:
  std::vector<Transform> transforms;
  std::vector<Texture> textures;
  void New(Transform transform, Texture texture);
  void Draw(Shader &shader, GetModelFlags model_flags);
  void Inspector();
  void Update();
  explicit Sprites(RenderSystem &rs) : render(rs) {}
};
