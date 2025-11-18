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
   0.5f,  0.5f, 1.0f, 0.0f,
   0.5f, -0.5f, 1.0f, 1.0f,
  -0.5f, -0.5f, 0.0f, 1.0f,
  -0.5f,  0.5f, 0.0f, 0.0f
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
  World world;
  Context::Get().render_sys.Init();
  Context::Get().physics_sys.Init();

  AttributePointer position_pointer {
    .size = 2,
    .type = DataType::FLOAT,
    .normalized = false,
    .stride = 4 * sizeof(float),
    .pointer = (void *)0
  };
  AttributePointer texture_pointer {
    .size = 2,
    .type = DataType::FLOAT,
    .normalized = false,
    .stride = 4 * sizeof(float),
    .pointer = (void *)(2 * sizeof(float))
  };
  DataDescriptor data_desc {
    .pointers = {position_pointer, texture_pointer},
    .element_enabled = true,
    .vertices = vertices,
    .indices = indices
  };
  auto data = Context::Get().render_sys.CreateData(data_desc);

  DataDescriptor screen_data_desc {
    .pointers = {position_pointer, texture_pointer},
    .element_enabled = true,
    .vertices = screen_vertices,
    .indices = indices
  };
  auto screen_data = Context::Get().render_sys.CreateData(screen_data_desc);

  auto framebuffer_info = FramebufferInfo{
      .recreate_on_resize = true
  };
  auto framebuffer = Context::Get().render_sys.CreateFramebuffer(framebuffer_info);

  auto camera_entity = world.CreateEntity();
  world.AddComponent<Camera>(camera_entity, Camera{});
  auto entity = world.CreateEntity();
  auto shader = Context::Get().asset_sys.LoadAsset<Shader>(
      "assets/sprite_shader.manifest", Context::Get());
  auto screen_shader = Context::Get().asset_sys.LoadAsset<Shader>(
      "assets/screen_shader.manifest", Context::Get());
  auto texture = Context::Get().asset_sys.LoadAsset<Texture>(
      "assets/test_texture.manifest", Context::Get());
  world.AddComponent<Transform>(entity, Transform{});
  world.AddComponent<Renderable>(entity, Renderable{texture, shader, data});
  auto light_entity = world.CreateEntity();
  world.AddComponent<Transform>(light_entity, Transform{glm::vec2(2.0f, 2.0f), 0, glm::vec2(1.0f), 0.0f});
  world.AddComponent<Light>(light_entity, Light{1.0f, 0.1f, 0.1f});

  while (!glfwWindowShouldClose(Context::Get().render_sys.GetWindow())) {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    Context::Get().physics_sys.Update();
    Context::Get().asset_sys.Inspector();

    // World Inspector - Due to be moved to a function soon
    {
      ImGui::Begin("World");
      auto& transforms = world.GetComponentSet<Transform>();
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
          if (world.ContainsComponent<Renderable>(id)) {
            if (ImGui::TreeNode("Renderable")) {
              auto renderable = world.GetComponent<Renderable>(id);
              ImGui::Image(renderable->texture->texture, ImVec2(100, 100));
              ImGui::TreePop();
            }
          }
          if (world.ContainsComponent<Camera>(id)) {
            if (ImGui::TreeNode("Camera")) {
              auto camera = world.GetComponent<Camera>(id);
              ImGui::Text("Camera Component");
              ImGui::TreePop();
            }
          }
          if (world.ContainsComponent<Light>(id)) {
            if (ImGui::TreeNode("Light")) {
              auto light = world.GetComponent<Light>(id);
              ImGui::ColorEdit3("Color", glm::value_ptr(light->color), 0.01f);
              ImGui::DragFloat("Intensity", &light->intensity, 0.01f);
              ImGui::DragFloat("Radial Falloff", &light->radial_falloff, 0.01f);
              // ImGui::DragFloat("Angular Falloff", &light->angular_falloff, 0.01f);
              ImGui::TreePop();
            }
          }
        }
        ImGui::PopID();
        ImGui::Separator();
      }
      ImGui::End();
    }

    ImGui::Render();
    Context::Get().render_sys.BindFramebuffer(framebuffer);
    Context::Get().render_sys.Clear({0.1f, 0.1f, 0.1f, 1.0f});
    Context::Get().render_sys.DrawWorld(world);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    Context::Get().render_sys.Render(framebuffer, screen_data, screen_shader);
  }
  Context::Get().physics_sys.Quit();
  Context::Get().render_sys.DestroyData(data);
  Context::Get().render_sys.DestroyShader(shader);
  Context::Get().render_sys.Quit();
  return 0;
}
