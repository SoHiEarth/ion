#pragma once

#include "component.h"
#include "render.h"
#include <map>
#include <filesystem>
#include <memory>

using EntityID = std::uint32_t;
const EntityID NULL_ENTITY = std::numeric_limits<EntityID>::max();

struct World {
private:
  EntityID next_id = 1;
  std::map<EntityID, std::string> markers{};
  std::map<EntityID, std::shared_ptr<Transform>> transforms{};
  std::map<EntityID, std::shared_ptr<Renderable>> renderables{};
  std::map<EntityID, std::shared_ptr<PhysicsBody>> physics_bodies{};
  std::map<EntityID, std::shared_ptr<Camera>> cameras{};
  std::map<EntityID, std::shared_ptr<Light>> lights{};
  std::map<EntityID, std::shared_ptr<Script>> scripts{};
  std::map<EntityID, std::shared_ptr<void*>> custom_components{};
	std::filesystem::path world_path;

public:
  World(std::filesystem::path path) : world_path(path) {}
	std::filesystem::path GetWorldPath() const { return world_path; }
  std::map<EntityID, std::string>& GetMarkers();
	// Creates a new entity and returns its ID
	// Note: Adds a transform component after creating an entity
  EntityID CreateEntity();
  void DestroyEntity(EntityID entity);
  EntityID GetNextEntityID() const { return next_id; }

  template <typename T> std::map<EntityID, std::shared_ptr<T>> &GetComponentSet();

  template <typename T> std::shared_ptr<T> NewComponent(EntityID entity) {
    GetComponentSet<T>().insert({entity, std::make_shared<T>()});
    return GetComponentSet<T>().at(entity);
	}

  template <typename T> std::shared_ptr<T> GetComponent(EntityID entity) {
    auto it = GetComponentSet<T>().find(entity);
    if (it != GetComponentSet<T>().end()) {
      return it->second;
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
