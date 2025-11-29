#include "world.h"

template <> std::map<EntityID, std::shared_ptr<Transform>> &World::GetComponentSet<Transform>() {
  return transforms;
}

template <>
std::map<EntityID, std::shared_ptr<Renderable>> &World::GetComponentSet<Renderable>() {
  return renderables;
}

template <>
std::map<EntityID, std::shared_ptr<PhysicsBody>> &World::GetComponentSet<PhysicsBody>() {
  return physics_bodies;
}

template <> std::map<EntityID, std::shared_ptr<Camera>> &World::GetComponentSet<Camera>() {
  return cameras;
}

template <> std::map<EntityID, std::shared_ptr<Light>> &World::GetComponentSet<Light>() {
  return lights;
}

template <> std::map<EntityID, std::shared_ptr<Script>> &World::GetComponentSet<Script>() {
  return scripts;
}

template <> std::map<EntityID, std::shared_ptr<void>>& World::GetComponentSet<void>() {
  return custom_components;
}

EntityID World::CreateEntity() {
  auto id = next_id++;
	NewComponent<Transform>(id);
	return id;
}

void World::DestroyEntity(EntityID entity) {
  transforms.erase(entity);
  renderables.erase(entity);
  physics_bodies.erase(entity);
  cameras.erase(entity);
  lights.erase(entity);
}
