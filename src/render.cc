// dependency
#include <glad/glad.h>
// end
#include "component.h"
#include "error_code.h"
#include "render.h"
#include "shader.h"
#include "texture.h"
#include "world.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "context.h"
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <stb_image.h>

glm::vec2 window_size = glm::vec2(800, 600);

void SizeCallback(GLFWwindow *window, int w, int h) {
  glViewport(0, 0, w, h);
  window_size.x = w;
  window_size.y = h;
  Context::Get().render_sys;
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
  glViewport(0, 0, window_size.x, window_size.y);
  return 0;
}

GLFWwindow *RenderSystem::GetWindow() { return window; }
glm::vec2 RenderSystem::GetWindowSize() { return window_size; }

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

std::shared_ptr<GPUData> RenderSystem::CreateData(DataDescriptor &data_desc) {
  auto data = std::make_shared<GPUData>();
  data->element_enabled = data_desc.element_enabled;
  glGenVertexArrays(1, &data->vertex_attrib);
  glGenBuffers(1, &data->vertex_buffer);
  if (data->element_enabled) {
    glGenBuffers(1, &data->element_buffer);
  }
  glBindVertexArray(data->vertex_attrib);
  glBindBuffer(GL_ARRAY_BUFFER, data->vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data_desc.vertices.size(),
               data_desc.vertices.data(), GL_STATIC_DRAW);
  if (data->element_enabled) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->element_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(unsigned int) * data_desc.indices.size(),
                 data_desc.indices.data(), GL_STATIC_DRAW);
  }
  for (int i = 0; i < data_desc.pointers.size(); i++) {
    auto &pointer_data = data_desc.pointers[i];
    glVertexAttribPointer(i, pointer_data.size, GetTypeEnum(pointer_data.type),
                          pointer_data.normalized, pointer_data.stride,
                          pointer_data.pointer);
    glEnableVertexAttribArray(i);
  }
  UnbindData();
  return data;
}

void RenderSystem::DestroyData(std::shared_ptr<GPUData> data) {
  glDeleteVertexArrays(1, &data->vertex_attrib);
  glDeleteBuffers(1, &data->vertex_buffer);
  if (data->element_enabled) {
    glDeleteBuffers(1, &data->element_buffer);
  }
}

void RenderSystem::BindData(std::shared_ptr<GPUData> data) {
  glBindVertexArray(data->vertex_attrib);
  glBindBuffer(GL_ARRAY_BUFFER, data->vertex_buffer);
  if (data->element_enabled) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->element_buffer);
  }
}

void RenderSystem::UnbindData() {
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RenderSystem::DestroyShader(std::shared_ptr<Shader> shader) {
  glDeleteProgram(shader->GetProgram());
  shader.reset();
}

void RenderSystem::Clear(glm::vec4 color) {
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);
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

glm::mat4 GetModelFromTransform(Transform transform) {
  auto model = glm::mat4(1.0f);
  model = glm::translate(
      model, glm::vec3(transform.position, transform.layer * 0.01f));
  model = glm::rotate(model, glm::radians(transform.rotation),
                      glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::scale(model, glm::vec3(transform.scale, 1));
  return model;
}

void RenderSystem::DrawWorld(World &world) {
  auto &all_cameras = world.GetComponentSet<Camera>();
  auto &all_transforms = world.GetComponentSet<Transform>();
  for (auto &[entity_id, camera] : all_cameras) {
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, -glm::vec3{camera.position, 3.0});
    glm::mat4 projection;
    // if (camera.mode == ProjectionMode::PERSPECTIVE) {
    projection =
        glm::perspective(glm::radians(45.0f),
                         GetWindowSize().x / GetWindowSize().y, 0.1f, 100.0f);
    /*
    } else {
    float ortho_scale = 10.0f;
    projection = glm::ortho(-ortho_scale * (render_sys.GetWindowSize().x /
                                            render_sys.GetWindowSize().y),
                            ortho_scale * (render_sys.GetWindowSize().x /
                                           render_sys.GetWindowSize().y),
                            -ortho_scale, ortho_scale, 0.1f, 100.0f);
    } */
    for (auto &[entity_id, transform] : all_transforms) {
      auto renderable = world.GetComponent<Renderable>(entity_id);
      if (renderable) {
        BindData(renderable->data);
        renderable->shader->Use();
        renderable->shader->SetUniform("view", view);
        renderable->shader->SetUniform("projection", projection);
        auto model = GetModelFromTransform(transform);
        renderable->shader->SetUniform("model", model);
        glBindTexture(GL_TEXTURE_2D, renderable->texture->texture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        UnbindData();
      }
    }
  }
}

unsigned int RenderSystem::ConfigureTexture(TextureInfo texture_info) {
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  if (texture_info.data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_info.width,
                 texture_info.height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 texture_info.data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cerr << TEXTURE_LOAD_FAIL << std::endl;
  }
  return texture;
}

Framebuffer RenderSystem::CreateFramebuffer(FramebufferInfo& info) {
  Framebuffer framebuffer;
  framebuffer.recreate_on_resize = info.recreate_on_resize;
  glGenFramebuffers(1, &framebuffer.framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.framebuffer);
  glGenTextures(1, &framebuffer.colorbuffer);
  glBindTexture(GL_TEXTURE_2D, framebuffer.colorbuffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_size.x, window_size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.colorbuffer, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("Failed to create framebuffer\n");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  framebuffers.push_back(framebuffer);
  return framebuffer;
}

void RenderSystem::BindFramebuffer(Framebuffer framebuffer) {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.framebuffer);
}

void RenderSystem::UnbindFramebuffer() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
