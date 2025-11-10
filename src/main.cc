// Code block
#include <glad/glad.h>
// Code block
#include <GLFW/glfw3.h>
#include <array>
#include <cstdint>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <sstream>
#include <vector>

float vertices[] = {0.5f,  0.5f,  0.0f, 0.5f,  -0.5f, 0.0f,
                    -0.5f, -0.5f, 0.0f, -0.5f, 0.5f,  0.0f};

unsigned int indices[] = {0, 1, 3, 1, 2, 3};
float layer = 0;
glm::vec2 position = glm::vec2(0.0f), scale = glm::vec2(1.0f);
float rotation = 0.0f; // Radians
int screen_width = 800, screen_height = 600;

enum ErrorCodes : std::uint16_t {
  WINDOW_CREATE_FAIL = 1,
  OPENGL_LOADER_FAIL = 2,
  FILE_READ_FAIL = 3,
  VERTEX_COMPILATION_FAIL = 4,
  FRAGMENT_COMPILATION_FAIL = 5,
  SHADER_PROGRAM_LINK_FAIL = 6
};

std::string ReadFile(std::string path) {
  std::string data;
  std::ifstream file;
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    file.open(path);
    std::stringstream stream;
    stream << file.rdbuf();
    file.close();
    data = stream.str();
  } catch (std::ifstream::failure e) {
    std::cerr << FILE_READ_FAIL << std::endl;
  }
  return data;
}

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

  unsigned int vertex, fragment;
  int success;
  std::array<char, 512> info_log;
  vertex = glCreateShader(GL_VERTEX_SHADER);
  auto vertex_code = ReadFile("assets/sprite.vert");
  auto vertex_code_char = vertex_code.c_str();
  glShaderSource(vertex, 1, &vertex_code_char, nullptr);
  glCompileShader(vertex);
  glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex, 512, nullptr, info_log.data());
    std::cerr << VERTEX_COMPILATION_FAIL << std::endl;
  }
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  auto fragment_code = ReadFile("assets/sprite.frag");
  auto fragment_code_char = fragment_code.c_str();
  glShaderSource(fragment, 1, &fragment_code_char, nullptr);
  glCompileShader(fragment);
  glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment, 512, nullptr, info_log.data());
    std::cerr << FRAGMENT_COMPILATION_FAIL << std::endl;
  }
  auto program = glCreateProgram();
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, nullptr, info_log.data());
    std::cerr << SHADER_PROGRAM_LINK_FAIL << std::endl;
  }
  glDeleteShader(vertex);
  glDeleteShader(fragment);

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

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
      ImGui::Begin("Transform");
      ImGui::DragFloat2("Position", glm::value_ptr(position));
      ImGui::DragFloat("Layer", &layer);
      ImGui::DragFloat2("Scale", glm::value_ptr(scale));
      ImGui::DragFloat("Rotation", &rotation);
      ImGui::End();
    }
    ImGui::Render();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 model = glm::mat4(1.0f), view = glm::mat4(1.0f),
              projection = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position.x, position.y, layer));
    model =
        glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(scale.x, scale.y, 1));
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    projection = glm::perspective(glm::radians(45.0f),
                                  (float)screen_width / (float)screen_height,
                                  0.1f, 100.0f);
    auto model_location = glGetUniformLocation(program, "model"),
         view_location = glGetUniformLocation(program, "view"),
         projection_location = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projection_location, 1, GL_FALSE,
                       glm::value_ptr(projection));

    glUseProgram(program);
    glBindVertexArray(vertex_attrib);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }
  glDeleteVertexArrays(1, &vertex_attrib);
  glDeleteBuffers(1, &vertex_buffer);
  glDeleteProgram(program);
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
