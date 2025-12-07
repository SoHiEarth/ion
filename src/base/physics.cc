#include "ion/physics.h"
#include <box2d/box2d.h>

namespace ion::physics::internal {
  ION_API b2WorldId world = b2WorldId{};
} // namespace ion::physics::internal

b2WorldId ion::physics::GetWorld() {
  return internal::world;
}

void ion::physics::Init() {
  auto world_def = b2DefaultWorldDef();
  world_def.gravity = b2Vec2(0.0F, -1.0F);
  internal::world = b2CreateWorld(&world_def);
}

void ion::physics::Update() {
  b2World_Step(internal::world, 1.0F / 60.0F, 6);
}

void ion::physics::Quit() {
  if (b2World_IsValid(internal::world)) {
    b2DestroyWorld(internal::world);
    internal::world = b2WorldId{};
  }
}
