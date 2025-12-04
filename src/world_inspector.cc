#include "world_inspector.h"
#include "assets.h"
#include "texture.h"
#include "world.h"
#include "development/gui.h"
#include <format>
#include <glm/gtc/type_ptr.hpp>

void WorldInspector(std::shared_ptr<World> world, Defaults& defaults) {
  ImGui::Begin("World");
  if (ImGui::CollapsingHeader("Utilities", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::Button("Create Entity")) {
      world->CreateEntity();
    }

    static EntityID selected_entity;
    if (ImGui::Button("Add Component")) {
      ImGui::OpenPopup("Add Component");
      selected_entity = -1;
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
        renderable_component->color = defaults.default_textures->color;
        renderable_component->normal = defaults.default_textures->normal;
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
      if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat2("Position", glm::value_ptr(transform->position), 0.1f);
        ImGui::DragInt("Layer", &transform->layer);
        ImGui::DragFloat2("Scale", glm::value_ptr(transform->scale), 0.1f);
        ImGui::DragFloat("Rotation", &transform->rotation, 0.1f);
        ImGui::TreePop();
      }
      if (world->ContainsComponent<Renderable>(id)) {
        if (ImGui::TreeNode("Renderable")) {
          auto renderable = world->GetComponent<Renderable>(id);
          ImGui::Image(renderable->color->texture, ImVec2(100, 100));
          if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_ASSET")) {
              IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<Texture>));
              std::shared_ptr<Texture> dropped_texture = *(std::shared_ptr<Texture>*)payload->Data;
              renderable->color = dropped_texture;
            }
            ImGui::EndDragDropTarget();
          }
          ImGui::Image(renderable->normal->texture, ImVec2(100, 100));
          if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_ASSET")) {
              IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<Texture>));
              std::shared_ptr<Texture> dropped_texture = *(std::shared_ptr<Texture>*)payload->Data;
              renderable->normal = dropped_texture;
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
            ImGui::OpenPopup("AddParameterPopup");
          }
          bool add_parameter_open = true;
          if (ImGui::BeginPopupModal("AddParameterPopup", &add_parameter_open)) {
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