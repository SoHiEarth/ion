#include "ion/assets.h"
#include "ion/base_pipeline.h"
#include "ion/defaults.h"
#include "ion/development/package.h"
#include "ion/game/game.h"
#include "ion/physics.h"
#include "ion/render.h"
#include "ion/script.h"
#include "ion/shader.h"
#include "ion/systems.h"
#include "ion/texture.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include <pugixml.hpp>
#include <sstream>

std::map<int, std::filesystem::path> ReadWorldList(std::filesystem::path path) {
  if (!std::filesystem::exists(path)) {
    return {};
  }

  auto doc = pugi::xml_document{};
  auto result = doc.load_file(path.string().c_str());
  if (!result) {
    throw std::runtime_error(std::string("Failed to load world list: ") +
                             result.description());
  }

  std::map<int, std::filesystem::path> world_list{};
  auto root = doc.child("worlds");
  for (auto world_node : root.children("world")) {
    int id = world_node.attribute("id").as_int();
    std::string world_path_str = world_node.attribute("path").as_string();
    auto world_path = std::filesystem::path(world_path_str);
    world_list[id] = world_path;
  }
  return world_list;
}

static void Init() {
  ion::render::Init();
  ion::physics::Init();
  ion::script::Init();
}

static void RegisterAllSystems() {
  ion::systems::RegisterSystem(
      {"Script System", ion::systems::UpdatePhase::UPDATE,
       ion::systems::UpdateCondition::WHEN_PLAYING, ion::script::Update});
  ion::systems::RegisterSystem(
      {"Physics System", ion::systems::UpdatePhase::UPDATE,
       ion::systems::UpdateCondition::WHEN_PLAYING, ion::physics::Update});
  ion::systems::RegisterSystem(
      {"Game System", ion::systems::UpdatePhase::UPDATE,
       ion::systems::UpdateCondition::WHEN_PLAYING, ion::game::Update});
}

int main() {
  if (!ion::res::CheckApplicationStructure()) {
    printf("Invalid application structure. Exiting.\n");
    return -1;
  }

  auto world_list = ReadWorldList("world_list.cfg");
  printf("Loaded world list with %zu worlds.\n", world_list.size());
  if (world_list.empty()) {
    printf("No worlds to load. Exiting.\n");
    return 0;
  }

  try {
    Init();
    RegisterAllSystems();
  } catch (std::exception &e) {
    printf("Init Error: %s\n", e.what());
    return -1;
  }

  auto world = ion::res::LoadAsset<World>(world_list.begin()->second.string());
  ion::systems::SetState(true);

  auto pipeline_settings = PipelineSettings{};
  auto pipeline = BasePipeline{};
  auto defaults = Defaults{};

  while (!glfwWindowShouldClose(ion::render::GetWindow())) {
    glfwPollEvents();
    ion::systems::UpdateSystems(world, ion::systems::UpdatePhase::PRE_UPDATE);
    ion::systems::UpdateSystems(world, ion::systems::UpdatePhase::UPDATE);
    pipeline.Render(world, pipeline_settings);
    ion::render::Present();
    ion::systems::UpdateSystems(world, ion::systems::UpdatePhase::LATE_UPDATE);
  }

  ion::physics::Quit();
  ion::script::Quit();
  ion::render::Quit();
  return 0;
}