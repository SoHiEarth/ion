#include "world.h"

void World::Update() { sprites.Update(); }

void World::Draw(Shader &shader, GetModelFlags model_flags) {
  sprites.Draw(shader, model_flags);
}
