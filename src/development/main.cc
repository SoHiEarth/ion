#include "ion/assets.h"
#include "ion/base_pipeline.h"
#include "ion/development/gui.h"
#include "ion/development/inspector.h"
#include "ion/development/startwindow.h"
#include "ion/game/game.h"
#include "ion/physics.h"
#include "ion/render.h"
#include "ion/script.h"
#include "ion/systems.h"
#include "ion/world.h"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <tinyfiledialogs/tinyfiledialogs.h>

static void Init() {
  ion::render::Init();
  // ion::gui::Init(ion::render::GetWindow());
  // ION_GUI_PREP_CONTEXT();
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
    printf("Failed to verify application files.\n");
    return -1;
  }
  try {
    Init();
    RegisterAllSystems();
  } catch (std::exception &e) {
    printf("Init Error: %s\n", e.what());
    return -1;
  }

  std::shared_ptr<World> world = std::make_shared<World>("a");

  try {
    // world = ion::gui::StartWindow();
  } catch (std::exception &e) {
    printf("Start Window Error: %s\n", e.what());
    return -1;
  }

  try {
    auto pipeline_settings = PipelineSettings{};
    pipeline_settings.render_to_output_buffer = true;
    auto pipeline = BasePipeline{};
    auto defaults = Defaults{};

    while (!glfwWindowShouldClose(ion::render::GetWindow())) {
      glfwPollEvents();
      ion::systems::UpdateSystems(world, ion::systems::UpdatePhase::PRE_UPDATE);
      // ion::gui::NewFrame();
      //      ion::dev::ui::RenderInspector(world, defaults, pipeline,
      //                                    pipeline_settings);
      ion::systems::UpdateSystems(world, ion::systems::UpdatePhase::UPDATE);
      pipeline.Render(world, pipeline_settings);
      ion::render::UnbindFramebuffer();
      ion::render::Clear();
      // ion::gui::Render();
      ion::render::Present();
      ion::systems::UpdateSystems(world,
                                  ion::systems::UpdatePhase::LATE_UPDATE);
    }
  } catch (std::exception &e) {
    printf("Runtime Error: %s\n", e.what());
  }

  ion::physics::Quit();
  ion::script::Quit();
  ion::render::Quit();
  // ion::gui::Quit();
  return 0;
}
