#include "ion/development/inspector.h"
#include "ion/assets.h"
#include "ion/texture.h"
#include "ion/world.h"
#include "ion/development/gui.h"
#include <imgui_stdlib.h>
#include <format>
#include <glm/gtc/type_ptr.hpp>
#include "ion/shader.h"
#include "ion/development/package.h"
#include "ion/development/id.h"
#include <tinyfiledialogs/tinyfiledialogs.h>

namespace ion::dev::ui::internal {
  bool render_settings_open = false;
  bool asset_inspector_open = false;
}

std::vector<std::string> SplitString(const std::string& str, char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(str);
  while (std::getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

void MainMenuBar() {
  ImGui::BeginMainMenuBar();
  if (ImGui::BeginMenu("File")) {
    if (ImGui::MenuItem("Exit")) {
			glfwSetWindowShouldClose(ion::render::GetWindow(), GLFW_TRUE);
    }
		ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("View")) {
    if (ImGui::MenuItem("Render Settings")) {
      ion::dev::ui::internal::render_settings_open = true;
    }
    if (ImGui::MenuItem("Asset Inspector")) {
      ion::dev::ui::internal::asset_inspector_open = true;
		}
		ImGui::EndMenu();
  }
  ImGui::EndMainMenuBar();
}

void WorldInspector(std::shared_ptr<World>& world, Defaults& defaults) {
  ImGui::Begin("World");
  static std::map<int, std::filesystem::path> all_worlds;
  if (ImGui::CollapsingHeader("IO", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::CollapsingHeader("All Worlds", ImGuiTreeNodeFlags_DefaultOpen)) {
      bool current_world_found = false;
      for (auto& [index, path] : all_worlds) {
        if (path == world->GetWorldPath()) current_world_found = true;
        auto path_str = path.string();
        if (ImGui::InputText(std::format("{}", index).c_str(), &path_str)) {
          path = path_str;
        }
      }
      if (!current_world_found) {
        all_worlds.insert({ static_cast<int>(all_worlds.size()), world->GetWorldPath() });
      }
    }
    if (ImGui::Button("Load")) {
      auto path = tinyfd_openFileDialog("Open World", nullptr, 0, nullptr, nullptr, false);
      if (path) {
        auto new_world = ion::res::LoadAsset<World>(path, false);
        std::swap(world, new_world);
      }
    }
		ImGui::SameLine();
    if (ImGui::Button("Save")) {
      auto path = tinyfd_saveFileDialog("Save World", nullptr, 0, nullptr, nullptr);
      if (path) {
				ion::res::SaveAsset(path, world);
      }
		}
    ImGui::SameLine();
    if (ImGui::Button("Package & Ship")) {
      auto path = tinyfd_selectFolderDialog("Select Output Directory", nullptr);
      if (path) {
        auto package_data = ion::dev::PackageData{
          all_worlds,
          path
        };
        ion::dev::Packer::CreatePackaged(package_data);
      }
    }
  }

  if (ImGui::CollapsingHeader("Utilities", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::Button("Create Entity")) {
      world->CreateEntity();
    }

    static EntityID selected_entity;
    ImGui::SameLine();
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
      for (auto& [id, transform] : world->GetComponentSet<Transform>()) {
        if (ImGui::Selectable(std::format("Entity {}", id).c_str(), selected_entity == id, ImGuiSelectableFlags_DontClosePopups)) {
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
      for (auto& [id, transform] : world->GetComponentSet<Transform>()) {
        if (ImGui::Selectable(std::format("Entity {}", id).c_str(), selected_entity == id, ImGuiSelectableFlags_DontClosePopups)) {
          selected_entity = id;
        }
      }

      ImGui::SeparatorText("Select Component");
      if (ImGui::Selectable("Physics Body") && selected_entity != -1) {
        world->NewComponent<PhysicsBody>(selected_entity);
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::Selectable("Renderable") && selected_entity != -1) {
        auto renderable_component = world->NewComponent<Renderable>(selected_entity);
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

  auto& transforms = world->GetComponentSet<Transform>();
  for (auto& [id, transform] : transforms) {
    ImGui::PushID(id);
    if (ImGui::CollapsingHeader(std::format("Entity {}", id).c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			if (world->GetMarkers().contains(id)) {
				ImGui::InputText("Marker", &world->GetMarkers()[id]);
      }

      if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat2("Position", glm::value_ptr(transform->position), 0.1f);
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
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_ASSET")) {
              IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<Texture>));
              std::shared_ptr<Texture> dropped_texture = *(std::shared_ptr<Texture>*)payload->Data;
              renderable->color = dropped_texture;
            }
            ImGui::EndDragDropTarget();
          }
					ImGui::Text("Normal Texture");
          if (!renderable->normal) {
            ImGui::Text("None");
          }
          else {
            ImGui::Image(renderable->normal->texture, ImVec2(100, 100));
          }
          if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_ASSET")) {
              IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<Texture>));
              std::shared_ptr<Texture> dropped_texture = *(std::shared_ptr<Texture>*)payload->Data;
              renderable->normal = dropped_texture;
            }
            ImGui::EndDragDropTarget();
          }
					ImGui::Text("Shader: %s", renderable->shader ? "Loaded" : "None");
          if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHADER_ASSET")) {
              IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<Shader>));
              std::shared_ptr<Shader> dropped_shader = *(std::shared_ptr<Shader>*)payload->Data;
              renderable->shader = dropped_shader;
            }
            ImGui::EndDragDropTarget();
          }
					ImGui::Text("GPU Data: %s", renderable->data ? "Loaded" : "None");
          if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GPU_DATA_ASSET")) {
              IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<GPUData>));
              std::shared_ptr<GPUData> dropped_data = *(std::shared_ptr<GPUData>*)payload->Data;
              renderable->data = dropped_data;
            }
            ImGui::EndDragDropTarget();
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
					const char* light_types[] = { "Global", "Point" };
					int current_type = static_cast<int>(light->type);
					if (ImGui::Combo("Type", &current_type, light_types, IM_ARRAYSIZE(light_types))) {
						light->type = static_cast<LightType>(current_type);
					}
          ImGui::ColorEdit3("Color", glm::value_ptr(light->color), 0.01f);
          ImGui::DragFloat("Intensity", &light->intensity, 0.01f);
          ImGui::DragFloat("Radial Falloff", &light->radial_falloff, 0.01f);
					ImGui::DragFloat("Volumetric Intensity", &light->volumetric_intensity, 0.01f);
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
          for (auto& [key, value] : script->parameters) {
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

void AssetInspector(std::shared_ptr<World>& world) {
  ImGui::Begin("Asset System");
	ImGui::SeparatorText("Textures");
  if (ImGui::Button("Load Image")) {
    auto file_char = tinyfd_openFileDialog("Load Image", nullptr,
      0, nullptr, nullptr, true);
    if (file_char) {
			for (auto& file : SplitString(file_char, '|')) {
        ion::res::LoadAsset<Texture>(std::string(file), false);
      }
    }
  }
  for (const auto& [id, texture] : ion::res::GetTextures()) {
    ImGui::PushID(id.c_str());
    ImGui::Image(ion::res::GetTextures().at(id)->texture, ImVec2(100, 100));
		if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
			ImGui::Text("Path: %s", texture->GetPath().string().c_str());
      ImGui::EndTooltip();
		}
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
      ImGui::SetDragDropPayload("TEXTURE_ASSET", &ion::res::GetTextures().at(id), sizeof(std::shared_ptr<Texture>&));
      ImGui::EndDragDropSource();
    }
    ImGui::PopID();
  }
	ImGui::SeparatorText("Shaders");
	if (ImGui::Button("Load Shader")) {
    auto file_char = tinyfd_openFileDialog("Load Shader", nullptr,
      0, nullptr, nullptr, false);
    if (file_char) {
      ion::res::LoadAsset<Shader>(std::filesystem::path(file_char), false);
    }
  }
	for (const auto& [id, shader] : ion::res::GetShaders()) {
    ImGui::Text("Shader: %s", id.c_str());
    if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
      ImGui::Text("Path: %s", shader->GetPath().string().c_str());
			ImGui::EndTooltip();
    }
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
      ImGui::SetDragDropPayload("SHADER_ASSET", &ion::res::GetShaders().at(id), sizeof(std::shared_ptr<Shader>&));
      ImGui::EndDragDropSource();
		}
  }
	ImGui::SeparatorText("GPU Data");
  if (ImGui::Button("Load GPU Data")) {
    auto file_char = tinyfd_openFileDialog("Load GPU Data", nullptr,
      0, nullptr, nullptr, false);
    if (file_char) {
      ion::res::LoadAsset<GPUData>(std::filesystem::path(file_char), false);
    }
	}
	for (const auto& [id, gpu_data] : ion::res::GetGPUData()) {
    ImGui::Text("GPU Data: %s", id.c_str());
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
      ImGui::SetDragDropPayload("GPU_DATA_ASSET", &ion::res::GetGPUData().at(id), sizeof(std::shared_ptr<GPUData>&));
      ImGui::EndDragDropSource();
    }
  }
	ImGui::SeparatorText("Worlds");
  if (ImGui::Button("Load World")) {
    auto file_char = tinyfd_openFileDialog("Load World", nullptr,
      0, nullptr, nullptr, false);
    if (file_char) {
      ion::res::LoadAsset<World>(std::filesystem::path(file_char), false);
    }
	}
	for (const auto& [id, selected_world] : ion::res::GetWorlds()) {
		if (ImGui::Selectable(std::format("World: {}", id).c_str())) {
      auto new_world = ion::res::LoadAsset<World>(world->GetWorldPath(), false);
			std::swap(world, new_world);
    }
  }
  ImGui::End();
}

void RenderSettingInspector() {
  ImGui::Begin("Render Settings");
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

void ion::dev::ui::RenderInspector(std::shared_ptr<World>& world, Defaults& defaults) {
  ION_GUI_PREP_CONTEXT();
  MainMenuBar();
  WorldInspector(world, defaults);
  if (internal::asset_inspector_open) {
    AssetInspector(world);
  }
  if (internal::render_settings_open) {
    RenderSettingInspector();
  }
}