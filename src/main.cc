#include "assets.h"
#include "component.h"
#include "physics.h"
#include "render.h"
#include "shader.h"
#include "texture.h"
#include "world.h"
#include <GLFW/glfw3.h>
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <vector>

std::vector<float> vertices = {0.5f, 0.5f,  0.0f, 1.0f,  1.0f,  0.5f, -0.5f,
                               0.0f, 1.0f,  0.0f, -0.5f, -0.5f, 0.0f, 0.0f,
                               0.0f, -0.5f, 0.5f, 0.0f,  0.0f,  1.0f};
std::vector<unsigned int> indices = {0, 1, 3, 1, 2, 3};
bool render_colliders = false;

int main(int argc, char **argv) {
  World world;
  RenderSystem render;
  PhysicsSystem physics;
  AssetSystem assets;

  render.Init();
  physics.Init();

  AttributePointer position_pointer;
  position_pointer.size = 3;
  position_pointer.type = DataType::FLOAT;
  position_pointer.normalized = false;
  position_pointer.stride = 5 * sizeof(float);
  position_pointer.pointer = (void *)0;
  AttributePointer texture_pointer;
  texture_pointer.size = 2;
  texture_pointer.type = DataType::FLOAT;
  texture_pointer.normalized = false;
  texture_pointer.stride = 5 * sizeof(float);
  texture_pointer.pointer = (void *)(3 * sizeof(float));
  DataDescriptor data_desc;
  data_desc.pointers = {position_pointer, texture_pointer};
  data_desc.element_enabled = true;
  data_desc.vertices = vertices;
  data_desc.indices = indices;
  auto data = render.CreateData(data_desc);

  auto camera_entity = world.CreateEntity();
  world.AddComponent<Camera>(camera_entity, Camera{});

  auto entity = world.CreateEntity();
  auto shader = assets.LoadAsset<Shader>("assets/sprite_shader.manifest");
  auto texture = assets.LoadAsset<Texture>("assets/test_texture.manifest");
  world.AddComponent<Transform>(entity, Transform{});
  world.AddComponent<Renderable>(entity, Renderable{texture, shader});

  while (!glfwWindowShouldClose(render.GetWindow())) {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    physics.Update();
    assets.Inspector();
    ImGui::Render();
    render.Clear({0.1f, 0.1f, 0.1f, 1.0f});
    render.BindData(data);
    render.DrawWorld(world, render);
    render.UnbindData();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    render.Render();
  }
  physics.Quit();
  render.DestroyData(data);
  render.DestroyShader(shader);
  render.Quit();
  return 0;
}
