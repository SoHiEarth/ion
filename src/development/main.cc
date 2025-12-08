#include "ion/assets.h"
#include "ion/physics.h"
#include "ion/texture.h"
#include "ion/render.h"
#include "ion/world.h"
#include "ion/script.h"
#include <tinyfiledialogs/tinyfiledialogs.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <vector>
#include <format>
#include "ion/shader.h"
#include "ion/development/inspector.h"
#include "ion/development/gui.h"
#include "ion/development/startwindow.h"
#include "ion/systems.h"
#include "ion/game/game.h"
#include "ion/base_pipeline.h"

constexpr int BLOOM_STRENGTH_MIN = 1;
constexpr int BLOOM_STRENGTH_MAX = 20;
constexpr ImVec2 FRAMEBUFFER_PREVIEW_SIZE = ImVec2(200, 200);
constexpr ImVec2 FRAMEBUFFER_UV_0 = ImVec2(0, 1);
constexpr ImVec2 FRAMEBUFFER_UV_1 = ImVec2(1, 0);

static void Init() {
  ion::render::Init();
  ion::gui::Init(ion::render::GetWindow());
  ION_GUI_PREP_CONTEXT();
  ion::physics::Init();
  ion::script::Init();
}

static void SettingsInspector(PipelineSettings& settings) {
  ImGui::Begin("Framebuffers");
  ImGui::Checkbox("Enable Bloom", &settings.bloom_enable);
  ImGui::SliderInt("Bloom Strength", &settings.bloom_strength, BLOOM_STRENGTH_MIN, BLOOM_STRENGTH_MAX);
  for (auto& [buffer, name] : ion::render::GetFramebuffers()) {
    ImGui::PushID(buffer->framebuffer);
    ImGui::Image(buffer->colorbuffer, FRAMEBUFFER_PREVIEW_SIZE, FRAMEBUFFER_UV_0, FRAMEBUFFER_UV_1);
    ImGui::SameLine();
    ImGui::TextUnformatted(name.c_str());
    ImGui::PopID();
  }
  ImGui::End();
}

static void RegisterAllSystems() {
  ion::systems::RegisterSystem({
    "Script System",
    ion::systems::UpdatePhase::UPDATE,
    ion::systems::UpdateCondition::WHEN_PLAYING,
    ion::script::Update
		});
  ion::systems::RegisterSystem({
    "Physics System",
    ion::systems::UpdatePhase::UPDATE,
    ion::systems::UpdateCondition::WHEN_PLAYING,
    ion::physics::Update
		});
  ion::systems::RegisterSystem({
    "Game System",
    ion::systems::UpdatePhase::UPDATE,
    ion::systems::UpdateCondition::WHEN_PLAYING,
    ion::game::Update
   });
}

int main() {
  if (!ion::res::CheckApplicationStructure()) {
    printf("Failed to verify application files.\n");
    return -1;
  }
  try {
    Init();
    RegisterAllSystems();
  }
  catch (std::exception& e) {
    printf("Init Error: %s\n", e.what());
		return -1;
  }

  std::shared_ptr<World> world = nullptr;

  try {
    world = ion::gui::StartWindow();
  }
  catch (std::exception& e) {
    printf("Start Window Error: %s\n", e.what());
    return -1;
  }

  try {
    auto pipeline_settings = PipelineSettings{};
    auto pipeline = BasePipeline{};
    auto defaults = Defaults{};

    while (!glfwWindowShouldClose(ion::render::GetWindow())) {
      glfwPollEvents();
      ion::systems::UpdateSystems(world, ion::systems::UpdatePhase::PRE_UPDATE);
      ion::gui::NewFrame();
      SettingsInspector(pipeline_settings);
      ion::dev::ui::RenderInspector(world, defaults);
			ion::systems::UpdateSystems(world, ion::systems::UpdatePhase::UPDATE);
      pipeline.Render(world, pipeline_settings);
      ion::gui::Render();
      ion::render::Present();
			ion::systems::UpdateSystems(world, ion::systems::UpdatePhase::LATE_UPDATE);
    }
  }
  catch (std::exception& e) {
    printf("Runtime Error: %s\n", e.what());
	}

  ion::physics::Quit();
  ion::script::Quit();
  ion::render::Quit();
  ion::gui::Quit();
  return 0;
}