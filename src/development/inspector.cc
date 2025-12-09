#include "ion/development/inspector.h"
#include "ion/assets.h"
#include "ion/development/gui.h"
#include "ion/development/id.h"
#include "ion/development/package.h"
#include "ion/physics.h"
#include "ion/shader.h"
#include "ion/systems.h"
#include "ion/texture.h"
#include "ion/world.h"
#include <format>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_internal.h>
#include <map>
#include <tinyfiledialogs/tinyfiledialogs.h>
#include "ion/base_pipeline.h"

constexpr int BLOOM_STRENGTH_MIN = 1;
constexpr int BLOOM_STRENGTH_MAX = 20;
constexpr const char *WORLD_INSPECTOR_KEY = "World Inspector";
constexpr const char *ASSET_INSPECTOR_KEY = "Asset Inspector";
constexpr const char *RENDER_SETTINGS_KEY = "Render Settings";
constexpr const char *IO_INSPECTOR_KEY = "IO Settings";
constexpr const char *SYSTEM_INSPECTOR_KEY = "Systems";
constexpr const char *FRAMEBUFFER_INSPECTOR_KEY = "Framebuffers";
constexpr const char *VIEWPORT_INSPECTOR_KEY = "Viewport";
constexpr ImVec2 FRAMEBUFFER_PREVIEW_SIZE = ImVec2(200, 200);
constexpr ImVec2 FRAMEBUFFER_UV_0 = ImVec2(0, 1);
constexpr ImVec2 FRAMEBUFFER_UV_1 = ImVec2(1, 0);
constexpr float DOCK_LEFT_WIDTH= 0.3F;
constexpr float DOCK_RIGHT_WIDTH= 0.3F;
constexpr float DOCK_CENTER_WIDTH= 0.4F;

namespace ion::dev::ui::internal {
std::map<std::string, bool> inspector_state;
}

static std::vector<std::string> SplitString(const std::string &str,
                                            char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(str);
  while (std::getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

static std::map<int, std::filesystem::path> GetWorldPaths() {
  std::map<int, std::filesystem::path> world_paths;
  for (const auto &[id, world] : ion::res::GetWorlds()) {
    world_paths.insert({world_paths.size(), world->GetWorldPath()});
  }
  return world_paths;
}

static void MainMenuBar() {
  ImGui::BeginMainMenuBar();
  if (ImGui::BeginMenu("File")) {
    if (ImGui::MenuItem("Exit")) {
      glfwSetWindowShouldClose(ion::render::GetWindow(), GLFW_TRUE);
    }
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("View")) {
    for (auto &[key, state] : ion::dev::ui::internal::inspector_state) {
      if (ImGui::MenuItem(key.c_str())) {
        state = !state;
      }
    }
    ImGui::EndMenu();
  }
  ImGui::EndMainMenuBar();
}

static void IOInspector(std::shared_ptr<World> &world) {
  ImGui::Begin("IO");
  ImGui::SeparatorText("All Worlds");
  for (auto &[id, unloaded_world] : ion::res::GetWorlds()) {
    ImGui::Text("Path: %s", unloaded_world->GetWorldPath().c_str());
    ImGui::SameLine();
    if (ImGui::Button("Remove")) {
      ion::res::GetWorlds().erase(id);
      break;
    }
  }

  if (ImGui::Button("Load")) {
    auto path = tinyfd_openFileDialog("Open World", nullptr, 0, nullptr,
                                      nullptr, false);
    if (path) {
      auto new_world = ion::res::LoadAsset<World>(path, false);
      std::swap(world, new_world);
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Save")) {
    auto path =
        tinyfd_saveFileDialog("Save World", nullptr, 0, nullptr, nullptr);
    if (path) {
      ion::res::SaveAsset(path, world);
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Package")) {
    auto path = tinyfd_selectFolderDialog("Select Output Directory", nullptr);
    if (path) {
      auto package_data = ion::dev::PackageData{GetWorldPaths(), path};
      ion::dev::Packer::CreatePackaged(package_data);
    }
  }
  ImGui::End();
}

static void WorldInspector(std::shared_ptr<World> &world, Defaults &defaults) {
  ImGui::Begin("World");
  if (ImGui::CollapsingHeader("Utilities", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::Button("Create Entity")) {
      world->CreateEntity();
    }
    ImGui::SameLine();
    static EntityID selected_entity;
    if (ImGui::Button("Add Marker")) {
      ImGui::OpenPopup("Add Marker");
      selected_entity = -1;
    }

    ImGui::SameLine();
    if (ImGui::Button("Add Component")) {
      ImGui::OpenPopup("Add Component");
      selected_entity = -1;
    }

    if (ImGui::BeginPopupModal("Add Marker")) {
      ImGui::SeparatorText("Select Entity");
      for (auto &[id, transform] : world->GetComponentSet<Transform>()) {
        if (ImGui::Selectable(std::format("Entity {}", id).c_str(),
                              selected_entity == id,
                              ImGuiSelectableFlags_DontClosePopups)) {
          selected_entity = id;
        }
      }
      ImGui::SeparatorText("Enter Marker Name");
      static std::string name = "";
      ImGui::InputText("Marker Name", &name);
      if (ImGui::Button("Add") && selected_entity != -1 && !name.empty()) {
        world->GetMarkers()[selected_entity] = name;
        name.clear();
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::Button("Cancel")) {
        name.clear();
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Add Component")) {
      ImGui::SeparatorText("Select Entity");
      for (auto &[id, transform] : world->GetComponentSet<Transform>()) {
        if (ImGui::Selectable(std::format("Entity {}", id).c_str(),
                              selected_entity == id,
                              ImGuiSelectableFlags_DontClosePopups)) {
          selected_entity = id;
        }
      }

      ImGui::SeparatorText("Select Component");
      if (ImGui::Selectable("Physics Body") && selected_entity != -1) {
        world->NewComponent<PhysicsBody>(selected_entity)->body_id =
            ion::physics::CreateBody(
                world->GetComponent<Transform>(selected_entity));
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::Selectable("Renderable") && selected_entity != -1) {
        auto renderable_component =
            world->NewComponent<Renderable>(selected_entity);
        renderable_component->color = defaults.default_color;
        renderable_component->normal = defaults.default_normal;
        renderable_component->shader = defaults.default_shader;
        renderable_component->data = defaults.default_data;
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::Selectable("Light") && selected_entity != -1) {
        world->NewComponent<Light>(selected_entity);
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::Selectable("Script") && selected_entity != -1) {
        auto script_component = world->NewComponent<Script>(selected_entity);
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::Selectable("Camera") && selected_entity != -1) {
        auto script_component = world->NewComponent<Camera>(selected_entity);
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::Button("Cancel")) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
  }

  ImGui::SeparatorText("Entities");

  auto &transforms = world->GetComponentSet<Transform>();
  for (auto &[id, transform] : transforms) {
    ImGui::PushID(id);
    if (ImGui::CollapsingHeader(std::format("Entity {}", id).c_str(),
                                ImGuiTreeNodeFlags_DefaultOpen)) {
      if (world->GetMarkers().contains(id)) {
        ImGui::InputText("Marker", &world->GetMarkers()[id]);
        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
          world->GetMarkers().erase(id);
        }
      }

      if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat2("Position", glm::value_ptr(transform->position),
                          0.1f);
        ImGui::DragInt("Layer", &transform->layer, 1.0F, 0, 100);
        ImGui::DragFloat2("Scale", glm::value_ptr(transform->scale), 0.1f);
        ImGui::DragFloat("Rotation", &transform->rotation, 0.1f);
        ImGui::TreePop();
      }
      if (world->ContainsComponent<Renderable>(id)) {
        if (ImGui::TreeNode("Renderable")) {
          auto renderable = world->GetComponent<Renderable>(id);
          ImGui::Text("Color Texture");
          if (!renderable->color) {
            ImGui::Text("None");
          } else {
            ImGui::Image(renderable->color->texture, ImVec2(100, 100));
          }
          if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload *payload =
                    ImGui::AcceptDragDropPayload("TEXTURE_ASSET")) {
              IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<Texture>));
              std::shared_ptr<Texture> dropped_texture =
                  *(std::shared_ptr<Texture> *)payload->Data;
              renderable->color = dropped_texture;
            }
            ImGui::EndDragDropTarget();
          }
          ImGui::Text("Normal Texture");
          if (!renderable->normal) {
            ImGui::Text("None");
          } else {
            ImGui::Image(renderable->normal->texture, ImVec2(100, 100));
          }
          if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload *payload =
                    ImGui::AcceptDragDropPayload("TEXTURE_ASSET")) {
              IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<Texture>));
              std::shared_ptr<Texture> dropped_texture =
                  *(std::shared_ptr<Texture> *)payload->Data;
              renderable->normal = dropped_texture;
            }
            ImGui::EndDragDropTarget();
          }
          ImGui::Text("Shader: %s", renderable->shader ? "Loaded" : "None");
          if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload *payload =
                    ImGui::AcceptDragDropPayload("SHADER_ASSET")) {
              IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<Shader>));
              std::shared_ptr<Shader> dropped_shader =
                  *(std::shared_ptr<Shader> *)payload->Data;
              renderable->shader = dropped_shader;
            }
            ImGui::EndDragDropTarget();
          }
          ImGui::Text("GPU Data: %s", renderable->data ? "Loaded" : "None");
          if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload *payload =
                    ImGui::AcceptDragDropPayload("GPU_DATA_ASSET")) {
              IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<GPUData>));
              std::shared_ptr<GPUData> dropped_data =
                  *(std::shared_ptr<GPUData> *)payload->Data;
              renderable->data = dropped_data;
            }
            ImGui::EndDragDropTarget();
          }
          ImGui::TreePop();
        }
      }
      if (world->ContainsComponent<PhysicsBody>(id)) {
        if (ImGui::TreeNode("Physics Body")) {
          auto physics_body = world->GetComponent<PhysicsBody>(id);
          ImGui::Checkbox("Enabled", &physics_body->enabled);
          if (ion::physics::BodyIsValid(physics_body->body_id)) {
            ImGui::TextColored({0.0, 1.0, 0.0, 1.0}, "Body ID is Valid.");
          } else {
            ImGui::TextColored({1.0, 0.0, 0.0, 1.0}, "Body ID is invalid.");
          }
          ImGui::TreePop();
        }
      }
      if (world->ContainsComponent<Camera>(id)) {
        if (ImGui::TreeNode("Camera")) {
          auto camera = world->GetComponent<Camera>(id);
          ImGui::Text("Camera Component");
          ImGui::TreePop();
        }
      }
      if (world->ContainsComponent<Light>(id)) {
        if (ImGui::TreeNode("Light")) {
          auto light = world->GetComponent<Light>(id);
          // Light type combo box
          const char *light_types[] = {"Global", "Point"};
          int current_type = static_cast<int>(light->type);
          if (ImGui::Combo("Type", &current_type, light_types,
                           IM_ARRAYSIZE(light_types))) {
            light->type = static_cast<LightType>(current_type);
          }
          ImGui::ColorEdit3("Color", glm::value_ptr(light->color), 0.01f);
          ImGui::DragFloat("Intensity", &light->intensity, 0.01f);
          ImGui::DragFloat("Radial Falloff", &light->radial_falloff, 0.01f);
          ImGui::DragFloat("Volumetric Intensity", &light->volumetric_intensity,
                           0.01f);
          ImGui::TreePop();
        }
      }
      if (world->ContainsComponent<Script>(id)) {
        static std::string param_key;
        static std::string param_value;
        if (ImGui::TreeNode("Script")) {
          auto script = world->GetComponent<Script>(id);
          ImGui::InputText("Path", &script->path);
          ImGui::InputText("Module", &script->module_name);
          ImGui::Text("Parameters:");
          for (auto &[key, value] : script->parameters) {
            ImGui::Text("%s: %s", key.c_str(), value.c_str());
          }
          if (ImGui::Button("Add Parameter")) {
            param_key.clear();
            param_value.clear();
            ImGui::OpenPopup("Add Parameter");
          }
          bool add_parameter_open = true;
          if (ImGui::BeginPopupModal("Add Parameter", &add_parameter_open)) {
            ImGui::Text("Add Parameter");
            ImGui::InputText("Key", &param_key);
            ImGui::InputText("Value", &param_value);
            if (ImGui::Button("Add")) {
              if (!param_key.empty()) {
                script->parameters[param_key] = param_value;
              }
              ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Close"))
              ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
          }
          ImGui::TreePop();
        }
      }
    }
    ImGui::PopID();
    ImGui::Separator();
  }
  ImGui::End();
}

static void AssetInspector(std::shared_ptr<World> &world) {
  ImGui::Begin("Assets");
  ImGui::SeparatorText("Textures");
  if (ImGui::Button("Load Image")) {
    auto file_char =
        tinyfd_openFileDialog("Load Image", nullptr, 0, nullptr, nullptr, true);
    if (file_char) {
      for (auto &file : SplitString(file_char, '|')) {
        ion::res::LoadAsset<Texture>(std::string(file), false);
      }
    }
  }
  for (const auto &[id, texture] : ion::res::GetTextures()) {
    ImGui::PushID(id.c_str());
    ImGui::Image(ion::res::GetTextures().at(id)->texture, ImVec2(100, 100));
    if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      ImGui::Text("Path: %s", texture->GetPath().string().c_str());
      ImGui::EndTooltip();
    }
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
      ImGui::SetDragDropPayload("TEXTURE_ASSET",
                                &ion::res::GetTextures().at(id),
                                sizeof(std::shared_ptr<Texture> &));
      ImGui::EndDragDropSource();
    }
    ImGui::PopID();
  }
  ImGui::SeparatorText("Shaders");
  if (ImGui::Button("Load Shader")) {
    auto file_char = tinyfd_openFileDialog("Load Shader", nullptr, 0, nullptr,
                                           nullptr, false);
    if (file_char) {
      ion::res::LoadAsset<Shader>(std::filesystem::path(file_char), false);
    }
  }
  for (const auto &[id, shader] : ion::res::GetShaders()) {
    ImGui::Text("Shader: %s", id.c_str());
    if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      ImGui::Text("Path: %s", shader->GetPath().string().c_str());
      ImGui::EndTooltip();
    }
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
      ImGui::SetDragDropPayload("SHADER_ASSET", &ion::res::GetShaders().at(id),
                                sizeof(std::shared_ptr<Shader> &));
      ImGui::EndDragDropSource();
    }
  }
  ImGui::SeparatorText("GPU Data");
  if (ImGui::Button("Load GPU Data")) {
    auto file_char = tinyfd_openFileDialog("Load GPU Data", nullptr, 0, nullptr,
                                           nullptr, false);
    if (file_char) {
      ion::res::LoadAsset<GPUData>(std::filesystem::path(file_char), false);
    }
  }
  for (const auto &[id, gpu_data] : ion::res::GetGPUData()) {
    ImGui::Text("GPU Data: %s", id.c_str());
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
      ImGui::SetDragDropPayload("GPU_DATA_ASSET",
                                &ion::res::GetGPUData().at(id),
                                sizeof(std::shared_ptr<GPUData> &));
      ImGui::EndDragDropSource();
    }
  }
  ImGui::SeparatorText("Worlds");
  if (ImGui::Button("Load World")) {
    auto file_char = tinyfd_openFileDialog("Load World", nullptr, 0, nullptr,
                                           nullptr, false);
    if (file_char) {
      ion::res::LoadAsset<World>(std::filesystem::path(file_char), false);
    }
  }
  for (const auto &[id, selected_world] : ion::res::GetWorlds()) {
    if (ImGui::Selectable(std::format("World: {}", id).c_str())) {
      auto new_world = ion::res::LoadAsset<World>(world->GetWorldPath(), false);
      std::swap(world, new_world);
    }
  }
  ImGui::End();
}

static void RenderSettingInspector() {
  ImGui::Begin("Render");
  auto render_scale = ion::render::GetRenderScale();
  if (ImGui::DragInt("Render Scale", &render_scale, 1, 1, 16)) {
    ion::render::SetRenderScale(render_scale);
  }
  auto clear_color = ion::render::GetClearColor();
  if (ImGui::ColorEdit3("Clear Color", glm::value_ptr(clear_color), 0.01f)) {
    ion::render::SetClearColor(clear_color);
  }
  ImGui::End();
}

static void SystemInspector() {
  ImGui::Begin("Systems");
  auto playing = ion::systems::GetState();
  if (playing) {
    if (ImGui::Button("Pause")) {
      ion::systems::SetState(false);
    }
  } else {
    if (ImGui::Button("Play")) {
      ion::systems::SetState(true);
    }
  }
  ImGui::SeparatorText("Per-System State");
  for (auto &system : ion::systems::GetSystems()) {
    if (ImGui::Checkbox(system.name.c_str(), &system.enabled)) {
      ion::systems::SetSystemEnabled(system.name, system.enabled);
    }
    switch (system.phase) {
    case ion::systems::UpdatePhase::PRE_UPDATE:
      ImGui::Text("Phase: Pre-Update");
      break;
    case ion::systems::UpdatePhase::UPDATE:
      ImGui::Text("Phase: Update");
      break;
    case ion::systems::UpdatePhase::LATE_UPDATE:
      ImGui::Text("Phase: Late-Update");
      break;
    }
    switch (system.condition) {
    case ion::systems::UpdateCondition::ALWAYS:
      ImGui::Text("Condition: Always");
      break;
    case ion::systems::UpdateCondition::WHEN_PLAYING:
      ImGui::Text("Condition: When Playing");
      break;
    case ion::systems::UpdateCondition::WHEN_STOPPED:
      ImGui::Text("Condition: When Stopped");
      break;
    }
  }
  ImGui::End();
}

static void MinimalStateInspector() {
  ImGui::Begin("State", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);
	auto playing = ion::systems::GetState();
	if (playing) {
    if (ImGui::Button("Pause")) {
      ion::systems::SetState(false);
		}
    } else {
    if (ImGui::Button("Play")) {
      ion::systems::SetState(true);
		}
	}
  ImGui::End();
}

void FramebufferInspector(PipelineSettings& settings) {
  ImGui::Begin("Framebuffers");
  ImGui::Checkbox("Enable Bloom", &settings.bloom_enable);
  ImGui::SliderInt("Bloom Strength", &settings.bloom_strength,
    BLOOM_STRENGTH_MIN, BLOOM_STRENGTH_MAX);
  for (auto& [buffer, name] : ion::render::GetFramebuffers()) {
    ImGui::PushID(buffer->framebuffer);
    ImGui::Image(buffer->colorbuffer, FRAMEBUFFER_PREVIEW_SIZE,
      FRAMEBUFFER_UV_0, FRAMEBUFFER_UV_1);
    ImGui::SameLine();
    ImGui::TextUnformatted(name.c_str());
    ImGui::PopID();
  }
  ImGui::End();
}

void ViewportInspector(BasePipeline& pipeline) {
  ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoDecoration);
	auto window_size = ImGui::GetWindowSize();
  ImGui::Image(pipeline.output_buffer->colorbuffer, window_size,
		FRAMEBUFFER_UV_0, FRAMEBUFFER_UV_1);
  ImGui::End();
}

void ion::dev::ui::RenderInspector(std::shared_ptr<World> &world,
                                   Defaults &defaults, BasePipeline& pipeline, PipelineSettings& settings) {
  ION_GUI_PREP_CONTEXT();
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(viewport->Size);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0F);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0F);
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
    ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBackground;
  ImGui::Begin("DockSpace_Window", nullptr, window_flags);
  ImGui::PopStyleVar(2);
  ImGuiID dockspace_id = ImGui::GetID("DockSpace");
  ImGui::DockSpace(dockspace_id, ImVec2(0.0F, 0.0F));
  ImGui::End();
  static bool first_dock_layout = true;
  if (first_dock_layout) {
    first_dock_layout = false;
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);
    ImGuiID dock_main_id = dockspace_id;
    ImGuiID dock_id_top = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up,
			0.1F, nullptr, &dock_main_id);
    ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down,
      DOCK_CENTER_WIDTH, nullptr, &dock_main_id);
    ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, DOCK_LEFT_WIDTH,
      nullptr, &dock_main_id);
    ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right,
      DOCK_RIGHT_WIDTH, nullptr, &dock_main_id);
    ImGui::DockBuilderDockWindow("Viewport", dock_main_id);
    ImGui::DockBuilderDockWindow("World", dock_id_left);
    ImGui::DockBuilderDockWindow("Render", dock_id_right);
    ImGui::DockBuilderDockWindow("IO", dock_id_bottom);
    ImGui::DockBuilderDockWindow("Assets", dock_id_bottom);
    ImGui::DockBuilderDockWindow("Framebuffers", dock_id_bottom);
		ImGui::DockBuilderDockWindow("Systems", dock_id_bottom);
    ImGui::DockBuilderDockWindow("State", dock_id_top);
    ImGui::DockBuilderFinish(dockspace_id);
  }

  MainMenuBar();
  if (internal::inspector_state[WORLD_INSPECTOR_KEY]) {
    WorldInspector(world, defaults);
  }
  if (internal::inspector_state[ASSET_INSPECTOR_KEY]) {
    AssetInspector(world);
  }
  if (internal::inspector_state[RENDER_SETTINGS_KEY]) {
    RenderSettingInspector();
  }
  if (internal::inspector_state[SYSTEM_INSPECTOR_KEY]) {
    SystemInspector();
  }
  if (internal::inspector_state[IO_INSPECTOR_KEY]) {
    IOInspector(world);
  }
  if (internal::inspector_state[FRAMEBUFFER_INSPECTOR_KEY]) {
    FramebufferInspector(settings);
	}
  if (internal::inspector_state[VIEWPORT_INSPECTOR_KEY]) {
    ViewportInspector(pipeline);
	}
	MinimalStateInspector();
}