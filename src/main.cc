#include "context.h"
#include "render.h"
#include "physics.h"
#include "assets.h"
#include "world.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <vector>

std::vector<float> vertices = {  0.5f,  0.5f, 1.0f, 1.0f, 
                                 0.5f, -0.5f, 1.0f, 0.0f,
                                -0.5f, -0.5f, 0.0f, 0.0f,
                                -0.5f,  0.5f, 0.0f, 1.0f
                              };
std::vector<unsigned int> indices = {0, 1, 3, 1, 2, 3};

int main(int argc, char **argv) {
  World world;
  Context context;
  context.render_sys.Init();
  context.physics_sys.Init();

  AttributePointer position_pointer;
  position_pointer.size = 2;
  position_pointer.type = DataType::FLOAT;
  position_pointer.normalized = false;
  position_pointer.stride = 4 * sizeof(float);
  position_pointer.pointer = (void *)0;
  AttributePointer texture_pointer;
  texture_pointer.size = 2;
  texture_pointer.type = DataType::FLOAT;
  texture_pointer.normalized = false;
  texture_pointer.stride = 4 * sizeof(float);
  texture_pointer.pointer = (void *)(2 * sizeof(float));
  DataDescriptor data_desc;
  data_desc.pointers = {position_pointer, texture_pointer};
  data_desc.element_enabled = true;
  data_desc.vertices = vertices;
  data_desc.indices = indices;
  auto data = context.render_sys.CreateData(data_desc);

  auto camera_entity = world.CreateEntity();
  world.AddComponent<Camera>(camera_entity, Camera{});

  auto entity = world.CreateEntity();
  auto shader = context.asset_sys.LoadAsset<Shader>("assets/sprite_shader.manifest", context);
  auto texture = context.asset_sys.LoadAsset<Texture>("assets/test_texture.manifest", context);
  world.AddComponent<Transform>(entity, Transform{});
  world.AddComponent<Renderable>(entity, Renderable{texture, shader, data});

  while (!glfwWindowShouldClose(context.render_sys.GetWindow())) {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    context.physics_sys.Update();
    context.asset_sys.Inspector();
    ImGui::Render();
    context.render_sys.Clear({0.1f, 0.1f, 0.1f, 1.0f});
    context.render_sys.DrawWorld(world, context.render_sys);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    context.render_sys.Render();
  }
  context.physics_sys.Quit();
  context.render_sys.DestroyData(data);
  context.render_sys.DestroyShader(shader);
  context.render_sys.Quit();
  return 0;
}
