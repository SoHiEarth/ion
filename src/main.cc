#include "assets.h"
#include "context.h"
#include "physics.h"
#include "texture.h"
#include "render.h"
#include "world.h"
#include "script.h"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <vector>
#include <format>
#include "shader.h"
#include "development/gui.h"

std::vector<float> vertices = {
   0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
   0.5f, -0.5f,  0.0f,  1.0f,  1.0f,
  -0.5f, -0.5f,  0.0f,  0.0f,  1.0f,
  -0.5f,  0.5f,  0.0f,  0.0f,  0.0f
},
screen_vertices = {
   1.0f,  1.0f,  1.0f,  1.0f,
   1.0f, -1.0f,  1.0f,  0.0f,
  -1.0f, -1.0f,  0.0f,  0.0f,
  -1.0f,  1.0f,  0.0f,  1.0f
};

std::vector<unsigned int> indices = { 0, 1, 3, 1, 2, 3 };
bool bloom_enable = true;

int main(int argc, char **argv) {
  auto world = ion::GetSystem<AssetSystem>().LoadAsset<World>("world.manifest");
  ion::GetSystem<RenderSystem>().Init();
  ion::gui::Init(ion::GetSystem<RenderSystem>().GetWindow());
  ion::GetSystem<PhysicsSystem>().Init();
  ion::GetSystem<ScriptSystem>().Init();

  AttributePointer position_pointer {
    .size = 3,
    .type = DataType::FLOAT,
    .normalized = false,
    .stride = 5 * sizeof(float),
    .pointer = (void *)0
  };
  AttributePointer texture_pointer {
    .size = 2,
    .type = DataType::FLOAT,
    .normalized = false,
    .stride = 5 * sizeof(float),
    .pointer = (void *)(3 * sizeof(float))
  };
  DataDescriptor data_desc {
    .pointers = {position_pointer, texture_pointer},
    .element_enabled = true,
    .vertices = vertices,
    .indices = indices
  };
  auto data = ion::GetSystem<RenderSystem>().CreateData(data_desc);

  AttributePointer screen_position_pointer {
    .size = 2,
    .type = DataType::FLOAT,
    .normalized = false,
    .stride = 4 * sizeof(float),
    .pointer = (void *)0
	};
  AttributePointer screen_texture_pointer {
    .size = 2,
    .type = DataType::FLOAT,
    .normalized = false,
    .stride = 4 * sizeof(float),
    .pointer = (void *)(2 * sizeof(float))
	};
  DataDescriptor screen_data_desc {
    .pointers = {screen_position_pointer, screen_texture_pointer},
    .element_enabled = true,
    .vertices = screen_vertices,
    .indices = indices
  };
  auto screen_data = ion::GetSystem<RenderSystem>().CreateData(screen_data_desc);

  auto framebuffer_info = FramebufferInfo{
      .recreate_on_resize = true
  };

  framebuffer_info.name = "Color";
  auto color_buffer = ion::GetSystem<RenderSystem>().CreateFramebuffer(framebuffer_info);
  framebuffer_info.name = "Normal";
  auto normal_buffer = ion::GetSystem<RenderSystem>().CreateFramebuffer(framebuffer_info);
  framebuffer_info.name = "Shaded";
  auto shaded = ion::GetSystem<RenderSystem>().CreateFramebuffer(framebuffer_info);
  framebuffer_info.name = "Bloom";
  auto bloom_buffer = ion::GetSystem<RenderSystem>().CreateFramebuffer(framebuffer_info);
  framebuffer_info.name = "Bloom 2";
  auto bloom_buffer2 = ion::GetSystem<RenderSystem>().CreateFramebuffer(framebuffer_info);
	framebuffer_info.name = "Tonemapped";
	auto tonemap_buffer = ion::GetSystem<RenderSystem>().CreateFramebuffer(framebuffer_info);
  auto& final_framebuffer = shaded;

  auto deferred_shader = ion::GetSystem<AssetSystem>().LoadAsset<Shader>("assets/deferred_shader.manifest");
  auto screen_shader = ion::GetSystem<AssetSystem>().LoadAsset<Shader>("assets/screen_shader.manifest");
  auto bloom_shader = ion::GetSystem<AssetSystem>().LoadAsset<Shader>("assets/bloom_shader.manifest");
  auto bloom_blur_shader = ion::GetSystem<AssetSystem>().LoadAsset<Shader>("assets/bloom_blur_shader.manifest");
  auto combine_shader = ion::GetSystem<AssetSystem>().LoadAsset<Shader>("assets/bloom_combine_shader.manifest");
  auto tonemap_shader = ion::GetSystem<AssetSystem>().LoadAsset<Shader>("assets/tonemap_shader.manifest");

  auto default_texture = ion::GetSystem<AssetSystem>().LoadAsset<Texture>("assets/default_texture.manifest");
  auto default_shader = ion::GetSystem<AssetSystem>().LoadAsset<Shader>("assets/texture_shader.manifest");

  auto camera_entity = world->CreateEntity();
  world->NewComponent<Camera>(camera_entity);
  auto entity = world->CreateEntity();
  auto renderable = world->NewComponent<Renderable>(entity);
	renderable->color = default_texture;
  renderable->normal = default_texture;
	renderable->shader = default_shader;
	renderable->data = data;

  auto light_entity = world->CreateEntity();
	auto light = world->NewComponent<Light>(light_entity);
	light->intensity = 1.0f;
	light->radial_falloff = 1.0f;
	light->color = glm::vec3(1.0f, 1.0f, 1.0f);

  while (!glfwWindowShouldClose(ion::GetSystem<RenderSystem>().GetWindow())) {
    glfwPollEvents();
		ion::gui::NewFrame();
    ion::GetSystem<ScriptSystem>().Update(world);
    ion::GetSystem<PhysicsSystem>().Update();
    ion::GetSystem<AssetSystem>().Inspector();

    ION_GUI_PREP_CONTEXT();
    {
      ImGui::Begin("Framebuffers");
			ImGui::Checkbox("Enable Bloom", &bloom_enable);
      for (auto& [buffer, name] : ion::GetSystem<RenderSystem>().GetFramebuffers()) {
        ImGui::PushID(buffer->framebuffer);
        ImGui::Image(buffer->colorbuffer,
          ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::SameLine();
        ImGui::TextUnformatted(name.c_str());
        ImGui::SameLine();
				if (ImGui::Button("Set as Final")) {
          final_framebuffer = buffer;
        }
        ImGui::PopID();
      }

      ImGui::End();
    }

    // World Inspector - Due to be moved to a function soon
    {
      ImGui::Begin("World");
      if (ImGui::CollapsingHeader("Utilities", ImGuiTreeNodeFlags_DefaultOpen)) {
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
            renderable_component->color = default_texture;
            renderable_component->normal = default_texture;
            renderable_component->shader = default_shader;
            renderable_component->data = data;
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
      for (auto &[id, transform] : transforms) {
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
    
    ion::GetSystem<RenderSystem>().BindFramebuffer(color_buffer);
    ion::GetSystem<RenderSystem>().Clear({0.0f, 0.0f, 0.0f, 1.0f});
    ion::GetSystem<RenderSystem>().DrawWorld(world, RENDER_PASS_COLOR);
    ion::GetSystem<RenderSystem>().BindFramebuffer(normal_buffer);
    ion::GetSystem<RenderSystem>().Clear({0.0f, 0.0f, 0.0f, 1.0f});
    ion::GetSystem<RenderSystem>().DrawWorld(world, RENDER_PASS_NORMAL);

    ion::GetSystem<RenderSystem>().BindFramebuffer(shaded);
    ion::GetSystem<RenderSystem>().Clear({0.0f, 0.0f, 0.0f, 1.0f});
    ion::GetSystem<RenderSystem>().Render(color_buffer, normal_buffer, screen_data, deferred_shader, world);
    ion::GetSystem<RenderSystem>().UnbindFramebuffer();

    // Bloom
    if (bloom_enable) {
      ion::GetSystem<RenderSystem>().BindFramebuffer(bloom_buffer);
      ion::GetSystem<RenderSystem>().Clear({ 0.0f, 0.0f, 0.0f, 1.0f });
      ion::GetSystem<RenderSystem>().UseShader(bloom_shader);
      ion::GetSystem<RenderSystem>().RunPass(shaded, bloom_buffer, bloom_shader, screen_data);

      ion::GetSystem<RenderSystem>().UseShader(bloom_blur_shader);
      bool horizontal = true;
      int blur_amount = 10;

      for (int i = 0; i < blur_amount; i++) {
        auto& source = horizontal ? bloom_buffer : bloom_buffer2;
        auto& target = horizontal ? bloom_buffer2 : bloom_buffer;
        ion::GetSystem<RenderSystem>().BindFramebuffer(target);
        ion::GetSystem<RenderSystem>().Clear({ 0.0f, 0.0f, 0.0f, 1.0f });
        bloom_blur_shader->SetUniform("horizontal", (int)horizontal);
        ion::GetSystem<RenderSystem>().RunPass(source, target, bloom_blur_shader, screen_data);
        horizontal = !horizontal;
      }

      ion::GetSystem<RenderSystem>().UseShader(combine_shader);
      ion::GetSystem<RenderSystem>().BindTexture(shaded, 1);
      combine_shader->SetUniform("ION_PASS_FRAMEBUFFER", 1);
      ion::GetSystem<RenderSystem>().RunPass(bloom_buffer, shaded, combine_shader, screen_data);
    }

    //Context::Get().render_sys.UseShader(tonemap_shader);
    //Context::Get().render_sys.RunPass(tonemap_buffer, shaded, tonemap_shader, screen_data);

    ion::GetSystem<RenderSystem>().DrawFramebuffer(final_framebuffer, screen_shader, screen_data);
    ion::gui::Render();
    ion::GetSystem<RenderSystem>().Present();
  }
  ion::GetSystem<PhysicsSystem>().Quit();
  ion::GetSystem<RenderSystem>().DestroyData(screen_data);
  ion::GetSystem<RenderSystem>().DestroyData(data);
  ion::GetSystem<RenderSystem>().DestroyShader(screen_shader);
  ion::GetSystem<RenderSystem>().DestroyShader(deferred_shader);
  ion::GetSystem<RenderSystem>().DestroyShader(default_shader);
  ion::GetSystem<ScriptSystem>().Quit();
  ion::GetSystem<RenderSystem>().Quit();
  ion::gui::Quit();
  return 0;
}