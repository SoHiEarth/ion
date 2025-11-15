// dependency
#include <box2d/math_functions.h>
#include <glad/glad.h>
// end
#include "error_code.h"
#include "render.h"
#include "shader.h"
#include "texture.h"
#include "transform.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

glm::vec2 window_size = glm::vec2(800, 600);

void SizeCallback(GLFWwindow *window, int w, int h) {
  glViewport(0, 0, w, h);
  window_size.x = w;
  window_size.y = h;
}

int RenderSystem::Init() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  window = glfwCreateWindow(window_size.x, window_size.y, "ion", NULL, NULL);
  if (window == nullptr) {
    std::cerr << WINDOW_CREATE_FAIL << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 150");
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << OPENGL_LOADER_FAIL << std::endl;
    return -1;
  }
  glfwSetFramebufferSizeCallback(window, SizeCallback);
  glEnable(GL_DEPTH_TEST);
  glViewport(0, 0, window_size.x, window_size.y);
  return 0;
}

GLFWwindow *RenderSystem::GetWindow() { return window; }
glm::vec2 RenderSystem::GetWindowSize() { return window_size; }
void RenderSystem::SetMode(RenderMode mode) {
  switch (mode) {
  case RenderMode::FILL:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  case RenderMode::LINE:
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;
  }
}
void RenderSystem::RenderSprite(Transform transform, GetModelFlags model_flags,
                                Texture texture, Shader shader) {
  auto model = transform.GetModel(model_flags);
  shader.SetUniform("model", model);
  glBindTexture(GL_TEXTURE_2D, texture.texture);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

GLenum GetTypeEnum(DataType type) {
  switch (type) {
  case DataType::INT:
    return GL_INT;
    break;
  case DataType::UNSIGNED_INT:
    return GL_UNSIGNED_INT;
    break;
  case DataType::FLOAT:
    return GL_FLOAT;
    break;
  default:
    return GL_FLOAT;
    break;
  }
}

GPUData RenderSystem::CreateData(DataDescriptor data_desc) {
  GPUData data;
  data.element_enabled = data_desc.element_enabled;
  glGenVertexArrays(1, &data.vertex_attrib);
  glGenBuffers(1, &data.vertex_buffer);
  if (data.element_enabled) {
    glGenBuffers(1, &data.element_buffer);
  }
  glBindVertexArray(data.vertex_attrib);
  glBindBuffer(GL_ARRAY_BUFFER, data.vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data_desc.vertices.size(),
               data_desc.vertices.data(), GL_STATIC_DRAW);
  if (data.element_enabled) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.element_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(unsigned int) * data_desc.indices.size(),
                 data_desc.indices.data(), GL_STATIC_DRAW);
  }
  for (int i = 0; i < data_desc.pointers.size(); i++) {
    auto pointer_data = data_desc.pointers[i];
    glVertexAttribPointer(i, pointer_data.size, GetTypeEnum(pointer_data.type),
                          pointer_data.normalized, pointer_data.stride,
                          pointer_data.pointer);
    glEnableVertexAttribArray(i);
  }
  UnbindData();
  return data;
}

void RenderSystem::DestroyData(GPUData data) {
  glDeleteVertexArrays(1, &data.vertex_attrib);
  glDeleteBuffers(1, &data.vertex_buffer);
  if (data.element_enabled) {
    glDeleteBuffers(1, &data.element_buffer);
  }
}

void RenderSystem::BindData(GPUData data) {
  glBindVertexArray(data.vertex_attrib);
  glBindBuffer(GL_ARRAY_BUFFER, data.vertex_buffer);
  if (data.element_enabled) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.element_buffer);
  }
}

void RenderSystem::UnbindData() {
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RenderSystem::DestroyShader(Shader &shader) {
  glDeleteProgram(shader.GetProgram());
}

void RenderSystem::Clear(glm::vec4 color) {
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int RenderSystem::Render() {
  glfwSwapBuffers(window);
  return 0;
}

int RenderSystem::Quit() {
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
