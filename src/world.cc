#include "world.h"

template <> std::map<EntityID, Transform> &World::GetComponentSet<Transform>() {
  return transforms;
}

template <>
std::map<EntityID, Renderable> &World::GetComponentSet<Renderable>() {
  return renderables;
}

template <>
std::map<EntityID, PhysicsBody> &World::GetComponentSet<PhysicsBody>() {
  return physics_bodies;
}

template <> std::map<EntityID, Camera> &World::GetComponentSet<Camera>() {
  return cameras;
}

template <> std::map<EntityID, Light> &World::GetComponentSet<Light>() {
  return lights;
}

template <> std::map<EntityID, Script> &World::GetComponentSet<Script>() {
  return scripts;
}

EntityID World::CreateEntity() { return next_id++; }

void World::DestroyEntity(EntityID entity) {
  transforms.erase(entity);
  renderables.erase(entity);
  physics_bodies.erase(entity);
  cameras.erase(entity);
  lights.erase(entity);
}
