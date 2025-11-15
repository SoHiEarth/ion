#pragma once

#include "render.h"
#include "sprites.h"

struct World {
  Sprites sprites;
  void Update();
  void Draw(Shader &shader, GetModelFlags model_flags);
  World(RenderSystem rs) : sprites(rs) {}
};
