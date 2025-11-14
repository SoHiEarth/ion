// dependency
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

void RenderSystem::RenderSprite(Transform transform, GetModelFlags model_flags,
                                Texture texture, Shader shader) {
  auto model = transform.GetModel(model_flags);
  shader.SetUniform("model", model);
  glBindTexture(GL_TEXTURE_2D, texture.texture);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
