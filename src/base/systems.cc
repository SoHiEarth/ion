#include "ion/systems.h"

namespace ion::systems::internal {
std::vector<SystemInfo> systems;
bool is_playing = false;
} // namespace ion::systems::internal

void ion::systems::RegisterSystem(const SystemInfo &info) {
  internal::systems.push_back(info);
}

void ion::systems::UpdateSystems(std::shared_ptr<World> &world,
                                 UpdatePhase phase) {
  for (const auto &system : internal::systems) {
    if (system.phase == phase && system.enabled) {
      if (system.condition == UpdateCondition::ALWAYS ||
          (system.condition == UpdateCondition::WHEN_PLAYING &&
           internal::is_playing) ||
          (system.condition == UpdateCondition::WHEN_STOPPED &&
           !internal::is_playing)) {
        system.function(world);
      }
    }
  }
}

void ion::systems::SetSystemEnabled(const std::string &name, bool enabled) {
  for (auto &system : internal::systems) {
    if (system.name == name) {
      system.enabled = enabled;
      break;
    }
  }
}

std::vector<ion::systems::SystemInfo> &ion::systems::GetSystems() {
  return internal::systems;
}

bool ion::systems::GetState() { return internal::is_playing; }

void ion::systems::SetState(bool playing) { internal::is_playing = playing; }