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
#include <tinyfiledialogs/tinyfiledialogs.h>
#include <vector>

void Draw(Shader &shader, GetModelFlags model_flags = GetModelFlags::DEFAULT);
RenderSystem render;

std::vector<float> vertices = {0.5f, 0.5f,  0.0f, 1.0f,  1.0f,  0.5f, -0.5f,
                               0.0f, 1.0f,  0.0f, -0.5f, -0.5f, 0.0f, 0.0f,
                               0.0f, -0.5f, 0.5f, 0.0f,  0.0f,  1.0f};

std::vector<unsigned int> indices = {0, 1, 3, 1, 2, 3};
std::vector<Texture> textures;
bool render_colliders = false;
bool use_perspective = true;
glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, 3.0f);

int main(int argc, char **argv) {
  World world(render);

  render.Init();
  Shader shader = Shader("assets/sprite.vert", "assets/sprite.frag");
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

  b2WorldDef world_def = b2DefaultWorldDef();
  world_def.gravity = b2Vec2(0.0F, -1.0F);
  auto physics_world = b2CreateWorld(&world_def);

  while (!glfwWindowShouldClose(render.GetWindow())) {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    b2World_Step(physics_world, 1.0F / 60.0F, 6);
    world.Update();
    world.sprites.Inspector();
    ImGui::Begin("Control Panel");
    if (ImGui::Button("Add")) {
      world.sprites.New(Transform(physics_world), Texture());
    }
    ImGui::SeparatorText("Assets");
    if (ImGui::Button("Load")) {
      const auto file_char = tinyfd_openFileDialog("Open Image", nullptr, 0,
                                                   nullptr, nullptr, false);
      if (file_char) {
        textures.push_back(Texture(file_char));
      }
    }
    for (int i = 0; i < textures.size(); i++) {
      ImGui::PushID(i);
      ImGui::Image(textures[i].texture, ImVec2(100, 100));
      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        ImGui::SetDragDropPayload("TEXTURE_ASSET", &textures[i],
                                  sizeof(Texture));
        ImGui::EndDragDropSource();
      }
      ImGui::PopID();
    }
    ImGui::Checkbox("Render Colliders", &render_colliders);
    ImGui::Checkbox("Use Perspective", &use_perspective);
    ImGui::DragFloat3("Camera Position", glm::value_ptr(camera_position), 0.1f);
    ImGui::End();
    ImGui::Render();
    render.Clear({0.1f, 0.1f, 0.1f, 1.0f});
    glm::mat4 view = glm::translate(glm::mat4(1.0f), -camera_position);
    glm::mat4 proj = glm::mat4(1.0f);
    auto window_size = render.GetWindowSize();
    if (!use_perspective) {
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
  b2DestroyWorld(physics_world);
  render.DestroyData(data);
  render.DestroyShader(shader);
  render.Quit();
  return 0;
}
