#include "assets.h"
#include "context.h"
#include "physics.h"
#include "texture.h"
#include "render.h"
#include "world.h"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <vector>

std::vector<float> vertices = {
   0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
   0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
  -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
  -0.5f,  0.5f, 0.0f, 0.0f, 0.0f
},
screen_vertices = {
    1.0f,  1.0f,  1.0f, 1.0f,
    1.0f, -1.0f,  1.0f, 0.0f,
   -1.0f, -1.0f,  0.0f, 0.0f,
   -1.0f,  1.0f,  0.0f, 1.0f
};
;
std::vector<unsigned int> indices = {0, 1, 3, 1, 2, 3};

int main(int argc, char **argv) {
  auto world = Context::Get().asset_sys.LoadAsset<World>("world.manifest", Context::Get());
  Context::Get().render_sys.Init();
  Context::Get().physics_sys.Init();

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
  auto data = Context::Get().render_sys.CreateData(data_desc);

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
  auto screen_data = Context::Get().render_sys.CreateData(screen_data_desc);

  auto framebuffer_info = FramebufferInfo{
      .recreate_on_resize = true
  };
  auto color_buffer = Context::Get().render_sys.CreateFramebuffer(framebuffer_info);
  auto normal_buffer = Context::Get().render_sys.CreateFramebuffer(framebuffer_info);
  auto framebuffer = Context::Get().render_sys.CreateFramebuffer(framebuffer_info);

  auto camera_entity = world->CreateEntity();
  world->AddComponent<Camera>(camera_entity, Camera{});
  auto entity = world->CreateEntity();
  auto shader = Context::Get().asset_sys.LoadAsset<Shader>(
      "assets/texture_shader.manifest", Context::Get());
  auto deferred_shader = Context::Get().asset_sys.LoadAsset<Shader>(
      "assets/deferred_shader.manifest", Context::Get());
  auto screen_shader = Context::Get().asset_sys.LoadAsset<Shader>(
      "assets/screen_shader.manifest", Context::Get());
  auto texture = Context::Get().asset_sys.LoadAsset<Texture>(
      "assets/test_texture.manifest", Context::Get());
  auto default_texture = Context::Get().asset_sys.LoadAsset<Texture>(
      "assets/default_texture.manifest", Context::Get());
  auto default_shader = Context::Get().asset_sys.LoadAsset<Shader>(
      "assets/texture_shader.manifest", Context::Get());
  world->AddComponent<Transform>(entity, Transform{});
  world->AddComponent<Renderable>(entity, Renderable{texture, texture, shader, data});
  auto light_entity = world->CreateEntity();
  world->AddComponent<Transform>(light_entity, Transform{glm::vec2(0.0f), 0, glm::vec2(1.0f), 0.0f});
  world->AddComponent<Light>(light_entity, Light{1.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f)});

  while (!glfwWindowShouldClose(Context::Get().render_sys.GetWindow())) {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    Context::Get().physics_sys.Update();
    Context::Get().asset_sys.Inspector();

    {
      ImGui::Begin("Framebuffers");
      ImGui::Image(color_buffer->colorbuffer,
                   ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));
      ImGui::SameLine();
      ImGui::Image(normal_buffer->colorbuffer,
                    ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));
      ImGui::SameLine();
      ImGui::Image(framebuffer->colorbuffer,
                   ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));
      ImGui::End();
    }

    // World Inspector - Due to be moved to a function soon
    {
      ImGui::Begin("World");
      if (ImGui::CollapsingHeader("Utilities", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Add Light")) {
          auto new_entity = world->CreateEntity();
          world->AddComponent<Transform>(new_entity, Transform{});
          world->AddComponent<Light>(new_entity, Light{1.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f)});
        }
        if (ImGui::Button("Add Renderable")) {
          auto new_entity = world->CreateEntity();
          world->AddComponent<Transform>(new_entity, Transform{});
          world->AddComponent<Renderable>(new_entity, Renderable{default_texture, default_texture, default_shader, data});
				}
      }
      auto& transforms = world->GetComponentSet<Transform>();
      for (auto &[id, transform] : transforms) {
        ImGui::PushID(id);
        if (ImGui::CollapsingHeader("Object", ImGuiTreeNodeFlags_DefaultOpen)) {
          if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat2("Position", glm::value_ptr(transform.position), 0.1f);
            ImGui::DragInt("Layer", &transform.layer);
            ImGui::DragFloat2("Scale", glm::value_ptr(transform.scale), 0.1f);
            ImGui::DragFloat("Rotation", &transform.rotation, 0.1f);
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
        }
        ImGui::PopID();
        ImGui::Separator();
      }
      ImGui::End();
    }
    
    Context::Get().render_sys.BindFramebuffer(color_buffer);
    Context::Get().render_sys.Clear({0.0f, 0.0f, 0.0f, 1.0f});
    Context::Get().render_sys.DrawWorld(world, RENDER_PASS_COLOR);
    Context::Get().render_sys.BindFramebuffer(normal_buffer);
    Context::Get().render_sys.Clear({0.0f, 0.0f, 0.0f, 1.0f});
    Context::Get().render_sys.DrawWorld(world, RENDER_PASS_NORMAL);
    Context::Get().render_sys.BindFramebuffer(framebuffer);
    Context::Get().render_sys.Clear({0.0f, 0.0f, 0.0f, 1.0f});
    Context::Get().render_sys.Render(color_buffer, normal_buffer, screen_data, deferred_shader, world);
    Context::Get().render_sys.UnbindFramebuffer();
    Context::Get().render_sys.Clear({0.1f, 0.1f, 0.1f, 1.0f});
    Context::Get().render_sys.DrawFramebuffer(framebuffer, screen_shader, screen_data);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    Context::Get().render_sys.Present();
  }
  Context::Get().physics_sys.Quit();
  Context::Get().render_sys.DestroyData(data);
  Context::Get().render_sys.DestroyShader(shader);
  Context::Get().render_sys.Quit();
  return 0;
}
