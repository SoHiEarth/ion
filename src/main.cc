#include "assets.h"
#include "camera.h"
#include "physics.h"
#include "render.h"
#include "shader.h"
#include "sprites.h"
#include "texture.h"
#include "transform.h"
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

RenderSystem render;
PhysicsSystem physics;
AssetSystem assets;
Camera camera;
static const char *proj_names[] = {"Orthographic", "Perspective"};
std::vector<float> vertices = {0.5f, 0.5f,  0.0f, 1.0f,  1.0f,  0.5f, -0.5f,
                               0.0f, 1.0f,  0.0f, -0.5f, -0.5f, 0.0f, 0.0f,
                               0.0f, -0.5f, 0.5f, 0.0f,  0.0f,  1.0f};
std::vector<unsigned int> indices = {0, 1, 3, 1, 2, 3};
bool render_colliders = false;

int main(int argc, char **argv) {
  auto world = World(render);

  render.Init();
  auto shader = assets.LoadAsset<Shader>("assets/sprite_shader.manifest");
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

  physics.Init();

  while (!glfwWindowShouldClose(render.GetWindow())) {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    physics.Update();
    world.Update();
    world.sprites.Inspector();
    assets.Inspector();
    ImGui::Begin("Control Panel");
    if (ImGui::Button("Add")) {
      auto physics_world = physics.GetWorld();
      world.sprites.New(Transform(physics_world), Texture());
    }

    ImGui::Checkbox("Render Colliders", &render_colliders);
    int current_mode = static_cast<int>(camera.mode);
    if (ImGui::Combo("Projection Mode", &current_mode, proj_names, 2)) {
      camera.mode = static_cast<ProjectionMode>(current_mode);
    }
    ImGui::DragFloat3("Camera Position", glm::value_ptr(camera.position), 0.1f);
    ImGui::End();
    ImGui::Render();
    render.Clear({0.1f, 0.1f, 0.1f, 1.0f});
    glm::mat4 view = glm::translate(glm::mat4(1.0f), -camera.position);
    glm::mat4 proj = glm::mat4(1.0f);
    auto window_size = render.GetWindowSize();
    if (camera.mode == ProjectionMode::PERSPECTIVE) {
      proj = glm::ortho(-window_size.x / 2, window_size.x / 2,
                        -window_size.y / 2, window_size.y / 2, 0.1f, 100.0f);
    } else {
      proj = glm::perspective(glm::radians(45.0f),
                              window_size.x / window_size.y, 0.1f, 100.0f);
    }
    shader.SetUniform("view", view);
    shader.SetUniform("projection", proj);
    render.BindData(data);
    world.Draw(shader, GetModelFlags::DEFAULT);
    if (render_colliders) {
      render.SetMode(RenderMode::LINE);
      world.Draw(shader, GetModelFlags::IGNORE_Z);
      render.SetMode(RenderMode::FILL);
    }
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
