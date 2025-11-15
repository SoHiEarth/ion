#include "physics.h"
#include <box2d/id.h>

b2WorldId PhysicsSystem::GetWorld() { return world; }

void PhysicsSystem::Init() {
  auto world_def = b2DefaultWorldDef();
  world_def.gravity = b2Vec2(0.0F, -1.0F);
  world = b2CreateWorld(&world_def);
}

void PhysicsSystem::Update() { b2World_Step(world, 1.0F / 60.0F, 6); }

void PhysicsSystem::Quit() { b2DestroyWorld(world); }
