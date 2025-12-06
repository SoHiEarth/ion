#include "ion/physics.h"
#include <box2d/box2d.h>

b2WorldId PhysicsSystem::GetWorld() { return world; }

void PhysicsSystem::Init() {
  auto world_def = b2DefaultWorldDef();
  world_def.gravity = b2Vec2(0.0F, -1.0F);
  world = b2CreateWorld(&world_def);
}

void PhysicsSystem::Update() { b2World_Step(world, 1.0F / 60.0F, 6); }

void PhysicsSystem::Quit() {
  if (b2World_IsValid(world)) {
    b2DestroyWorld(world);
    world = b2WorldId{0, 0};
  }
}
