// Code block
#include <glad/glad.h>
// Code block
#include "error_code.h"
#include "shader.h"
#include "transform.h"
#include <GLFW/glfw3.h>
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <vector>

float vertices[] = {0.5f,  0.5f,  0.0f, 0.5f,  -0.5f, 0.0f,
                    -0.5f, -0.5f, 0.0f, -0.5f, 0.5f,  0.0f};

unsigned int indices[] = {0, 1, 3, 1, 2, 3};
std::vector<Transform> transforms;
int screen_width = 800, screen_height = 600;

int main(int argc, char **argv) {
  if ((argc - 1) % 2 != 0) {
    std::cout << "Warning: Number of arguments is not even" << std::endl;
  }
  std::vector<std::pair<std::string, std::string>> arguments;
  for (int i = 1; i < argc; i += 2) {
    if (argv[i] != nullptr) {
      if (i + 1 < argc) {
        if (argv[i + 1] != nullptr) {
          arguments.push_back({argv[i], argv[i + 1]});
        }
      }
    }
  }

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  auto window =
      glfwCreateWindow(screen_width, screen_height, "ion", NULL, NULL);
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
  glfwSetFramebufferSizeCallback(window, [](GLFWwindow *, int w, int h) {
    glViewport(0, 0, w, h);
    screen_width = w;
    screen_height = h;
  });
  glEnable(GL_DEPTH_TEST);
  glViewport(0, 0, screen_width, screen_height);

  Shader shader = Shader("assets/sprite.vert", "assets/sprite.frag");

  unsigned int vertex_attrib, vertex_buffer, element_buffer;
  glGenVertexArrays(1, &vertex_attrib);
  glGenBuffers(1, &vertex_buffer);
  glGenBuffers(1, &element_buffer);
  glBindVertexArray(vertex_attrib);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  b2WorldDef world_def = b2DefaultWorldDef();
  world_def.gravity = b2Vec2(0.0F, -1.0F);
  auto world = b2CreateWorld(&world_def);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    b2World_Step(world, 1.0F / 60.0F, 6);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    for (int i = 0; i < transforms.size(); i++) {
      transforms[i].UpdatePhysics();
      transforms[i].RenderInspector(i);
    }
    ImGui::Begin("Add");
    if (ImGui::Button("Add")) {
      transforms.push_back(Transform(world));
    }
    ImGui::End();
    ImGui::Render();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 view =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    glm::mat4 proj = glm::perspective(
        glm::radians(45.0f), (float)screen_width / (float)screen_height, 0.1f,
        100.0f);
    auto view_loc = glGetUniformLocation(shader.GetProgram(), "view"),
         proj_loc = glGetUniformLocation(shader.GetProgram(), "projection");
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(proj));

    glBindVertexArray(vertex_attrib);
    for (auto &transform : transforms) {
      glm::mat4 model = transform.GetModel();
      auto model_loc = glGetUniformLocation(shader.GetProgram(), "model");
      glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
      shader.Use();
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }
  glDeleteVertexArrays(1, &vertex_attrib);
  glDeleteBuffers(1, &vertex_buffer);
  glDeleteProgram(shader.GetProgram());
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
