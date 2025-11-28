#pragma once

#include "component.h"
#include "render.h"
#include <map>

using EntityID = std::uint32_t;
const EntityID NULL_ENTITY = std::numeric_limits<EntityID>::max();

struct World {
private:
  EntityID next_id = 1;
  std::map<EntityID, Transform> transforms;
  std::map<EntityID, Renderable> renderables;
  std::map<EntityID, PhysicsBody> physics_bodies;
  std::map<EntityID, Camera> cameras;
  std::map<EntityID, Light> lights;
	std::map<EntityID, Script> scripts;

public:
  EntityID CreateEntity();
  void DestroyEntity(EntityID entity);
  EntityID GetNextEntityID() const { return next_id; }

  template <typename T> std::map<EntityID, T> &GetComponentSet();

  template <typename T> T *AddComponent(EntityID entity, const T &component) {
    GetComponentSet<T>().insert({entity, component});
    return &GetComponentSet<T>().at(entity);
  }

  template <typename T> T *GetComponent(EntityID entity) {
    auto it = GetComponentSet<T>().find(entity);
    if (it != GetComponentSet<T>().end()) {
      return &it->second;
    }
    return nullptr;
  }

  template <typename T> bool ContainsComponent(EntityID entity) {
    return GetComponentSet<T>().contains(entity);
  }

  template <typename T> void RemoveComponent(EntityID entity) {
    GetComponentSet<T>().erase(entity);
  }
};
