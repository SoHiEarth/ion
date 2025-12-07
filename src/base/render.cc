// dependency
#include <glad/glad.h>
// end
#include "ion/component.h"
#include "ion/error_code.h"
#include "ion/render.h"
#include "ion/shader.h"
#include "ion/texture.h"
#include "ion/world.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stb_image.h>
#include <string>

namespace ion::render::internal {
	ION_API GLFWwindow* window = nullptr;
	ION_API std::map<std::shared_ptr<Framebuffer>, std::string> framebuffers;
} // namespace ion::render::internal

class RenderConfig {
  public:
  glm::vec2 window_size = glm::vec2(800, 600);
	int render_scale = 4;
	float camera_z = 3.0f;
	glm::vec3 clear_color = glm::vec3(0.0f, 0.0f, 0.0f);
};
static RenderConfig r_config;

static void SizeCallback(GLFWwindow* window, int w, int h) {
  r_config.window_size.x = w;
  r_config.window_size.y = h;
  ion::render::UpdateFramebuffers();
}
static glm::mat4 GetModelFromTransform(std::shared_ptr<Transform> transform) {
  auto model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(transform->position.x, transform->position.y, transform->layer));
  model = glm::rotate(model, glm::radians(transform->rotation), glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::scale(model, glm::vec3(transform->scale.x, transform->scale.y, 1));
  return model;
}
static GLenum GetTypeEnum(DataType type) {
  switch (type) {
  case DataType::INT:
    return GL_INT;
  case DataType::UNSIGNED_INT:
    return GL_UNSIGNED_INT;
  case DataType::FLOAT:
    return GL_FLOAT;
  default:
    return GL_FLOAT;
  }
}

int ion::render::Init() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  internal::window = glfwCreateWindow(
    r_config.window_size.x, r_config.window_size.y,
    "ion", NULL, NULL
  );
  if (!internal::window) {
    printf("%d\n", WINDOW_CREATE_FAIL);
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(internal::window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("%d\n", OPENGL_LOADER_FAIL);
    return -1;
  }
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  glfwSetFramebufferSizeCallback(internal::window, SizeCallback);
  return 0;
}
GLFWwindow* ion::render::GetWindow() {
  return internal::window;
}
glm::vec2 ion::render::GetWindowSize() {
  return r_config.window_size;
}
int ion::render::GetRenderScale() {
  return r_config.render_scale;
}
void ion::render::SetRenderScale(int scale) {
  r_config.render_scale = scale;
  UpdateFramebuffers();
}
glm::vec3 ion::render::GetClearColor() {
  return r_config.clear_color;
}
void ion::render::SetClearColor(glm::vec3 color) {
  r_config.clear_color = color;
}

void ion::render::ConfigureData(std::shared_ptr<GPUData> gpu_data) {
	auto desc = gpu_data->GetDescriptor();
  gpu_data->element_enabled = desc.element_enabled;
  glGenVertexArrays(1, &gpu_data->vertex_attrib);
  glGenBuffers(1, &gpu_data->vertex_buffer);
  if (gpu_data->element_enabled) {
    glGenBuffers(1, &gpu_data->element_buffer);
  }
  glBindVertexArray(gpu_data->vertex_attrib);
  glBindBuffer(GL_ARRAY_BUFFER, gpu_data->vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * desc.vertices.size(),
               desc.vertices.data(), GL_STATIC_DRAW);
  if (gpu_data->element_enabled) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu_data->element_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(unsigned int) * desc.indices.size(),
                 desc.indices.data(), GL_STATIC_DRAW);
  }
  for (int i = 0; i < desc.pointers.size(); i++) {
    auto &pointer_data = desc.pointers[i];
    glVertexAttribPointer(i, pointer_data.size, GetTypeEnum(pointer_data.type),
                          pointer_data.normalized, pointer_data.stride,
                          pointer_data.pointer);
    glEnableVertexAttribArray(i);
  }
  UnbindData();
}
void ion::render::DestroyData(std::shared_ptr<GPUData> data) {
  glDeleteVertexArrays(1, &data->vertex_attrib);
  glDeleteBuffers(1, &data->vertex_buffer);
  if (data->element_enabled) {
    glDeleteBuffers(1, &data->element_buffer);
  }
}
void ion::render::BindData(std::shared_ptr<GPUData> data) {
  glBindVertexArray(data->vertex_attrib);
  glBindBuffer(GL_ARRAY_BUFFER, data->vertex_buffer);
  if (data->element_enabled) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->element_buffer);
  }
}
void ion::render::UnbindData() {
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

unsigned int ion::render::ConfigureTexture(const TextureInfo& texture_info) {
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  if (texture_info.data) {
    GLenum format;
    switch (texture_info.nr_channels) {
    case 1:
      format = GL_RED;
      break;
    case 2:
      format = GL_RG;
      break;
    case 3:
      format = GL_RGB;
      break;
    case 4:
      format = GL_RGBA;
      break;
    default:
      format = GL_RGB;
      break;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, format, texture_info.width,
      texture_info.height, 0, format, GL_UNSIGNED_BYTE,
      texture_info.data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
  else {
    printf("%d\n", TEXTURE_LOAD_FAIL);
  }
  return texture;
}

std::shared_ptr<Framebuffer> ion::render::CreateFramebuffer(const FramebufferInfo& info) {
  auto framebuffer = std::make_shared<Framebuffer>();
  framebuffer->recreate_on_resize = info.recreate_on_resize;
  glGenFramebuffers(1, &framebuffer->framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->framebuffer);
  glGenTextures(1, &framebuffer->colorbuffer);
  glBindTexture(GL_TEXTURE_2D, framebuffer->colorbuffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, static_cast<int>(r_config.window_size.x) / r_config.render_scale, static_cast<int>(r_config.window_size.y) / r_config.render_scale, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer->colorbuffer, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("Failed to create framebuffer\n");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  internal::framebuffers.insert({ framebuffer, info.name });
  return framebuffer;
}
void ion::render::UpdateFramebuffers() {
  for (auto& [framebuffer, name] : internal::framebuffers) {
    if (framebuffer->recreate_on_resize) {
      glBindTexture(GL_TEXTURE_2D, framebuffer->colorbuffer);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, static_cast<int>(r_config.window_size.x) / r_config.render_scale, static_cast<int>(r_config.window_size.y) / r_config.render_scale, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }
}
void ion::render::BindFramebuffer(std::shared_ptr<Framebuffer> framebuffer) {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->framebuffer);
  glViewport(0, 0, static_cast<int>(r_config.window_size.x) / r_config.render_scale, static_cast<int>(r_config.window_size.y) / r_config.render_scale);
}
void ion::render::UnbindFramebuffer() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void ion::render::DrawFramebuffer(std::shared_ptr<Framebuffer> framebuffer, std::shared_ptr<Shader> shader, std::shared_ptr<GPUData> quad) {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, static_cast<int>(r_config.window_size.x), static_cast<int>(r_config.window_size.y));
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  shader->Use();
  BindData(quad);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, framebuffer->colorbuffer);
  shader->SetUniform("screen_texture", 0);
  if (quad->element_enabled) {
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  }
  else {
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }
  glEnable(GL_DEPTH_TEST);
  UnbindData();
}
const std::map<std::shared_ptr<Framebuffer>, std::string>& ion::render::GetFramebuffers() {
  return internal::framebuffers;
}

void ion::render::BindTexture(std::shared_ptr<Texture> texture, int slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, texture->texture);
}
void ion::render::BindTexture(std::shared_ptr<Framebuffer> framebuffer, int slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, framebuffer->colorbuffer);
}

void ion::render::UseShader(std::shared_ptr<Shader> shader) {
  glUseProgram(shader->GetProgram());
}
void ion::render::DestroyShader(std::shared_ptr<Shader> shader) {
  glDeleteProgram(shader->GetProgram());
  shader.reset();
}

void ion::render::DrawWorld(std::shared_ptr<World> world, RenderPass pass) {
  for (auto& [entity_id, camera] : world->GetComponentSet<Camera>()) {
    glm::mat4 view = GetModelFromTransform(world->GetComponent<Transform>(entity_id));
    view = glm::translate(view, glm::vec3{ 0.0, 0.0, -3.0 });
    float ortho_scale = 10.0f;
    auto projection = glm::ortho(-ortho_scale * (ion::render::GetWindowSize().x /
      ion::render::GetWindowSize().y),
      ortho_scale * (ion::render::GetWindowSize().x /
        ion::render::GetWindowSize().y),
      -ortho_scale, ortho_scale, 0.1f, 100.0f);
    for (auto& [entity_id, transform] : world->GetComponentSet<Transform>()) {
      auto renderable = world->GetComponent<Renderable>(entity_id);
      if (renderable) {
        if (!renderable->shader || !renderable->data ||
          !renderable->color || !renderable->normal) {
          continue;
        }
        BindData(renderable->data);
        renderable->shader->Use();
        renderable->shader->SetUniform("layer", transform->layer);
        renderable->shader->SetUniform("view", view);
        renderable->shader->SetUniform("projection", projection);
        renderable->shader->SetUniform("model", GetModelFromTransform(transform));
        glActiveTexture(GL_TEXTURE0);
        if (pass == RENDER_PASS_COLOR) {
          glBindTexture(GL_TEXTURE_2D, renderable->color->texture);
        }
        else if (pass == RENDER_PASS_NORMAL) {
          glBindTexture(GL_TEXTURE_2D, renderable->normal->texture);
        }
        renderable->shader->SetUniform("sample", 0);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        UnbindData();
      }
    }
  }
}
void ion::render::RunPass(std::shared_ptr<Framebuffer> in, std::shared_ptr<Framebuffer> out, std::shared_ptr<Shader> shader, std::shared_ptr<GPUData> quad) {
  BindFramebuffer(out);
  shader->SetUniform("ION_PASS_IN", 0);
  BindTexture(in, 0);
  BindData(quad);
  if (quad->element_enabled) {
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  }
  else {
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }
  UnbindData();
}

void ion::render::Clear() {
  glClearColor(r_config.clear_color.r, r_config.clear_color.g, r_config.clear_color.b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}
void ion::render::Clear(glm::vec4 color) {
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);
}
int ion::render::Render(std::shared_ptr<Framebuffer> color_fb, std::shared_ptr<Framebuffer> normal_fb, std::shared_ptr<GPUData> quad, std::shared_ptr<Shader> shader, std::shared_ptr<World> world) {
  shader->Use();
  glClear(GL_COLOR_BUFFER_BIT);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, color_fb->colorbuffer);
  shader->SetUniform("color_texture", 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, normal_fb->colorbuffer);
  shader->SetUniform("normal_texture", 1);
  shader->SetUniform("light_count", static_cast<int>(world->GetComponentSet<Light>().size()));
  
  auto view = glm::mat4(1.0f);
  float ortho_scale = 10.0f;
  auto projection = glm::ortho(-ortho_scale * (GetWindowSize().x / GetWindowSize().y),
    ortho_scale * (GetWindowSize().x / GetWindowSize().y),
    -ortho_scale, ortho_scale, 0.1f, 100.0f);
  for (auto& [id, camera] : world->GetComponentSet<Camera>()) {
    view = GetModelFromTransform(world->GetComponent<Transform>(id));
    view = glm::translate(view, glm::vec3(0.0, 0.0, -3.0));
  }
  
  for (int i = 0; i < world->GetComponentSet<Light>().size(); i++) {
    auto& [entity_id, light] = *std::next(world->GetComponentSet<Light>().begin(), i);
    auto transform = world->GetComponent<Transform>(entity_id);
    glm::vec3 light_world_pos = glm::vec3(transform->position, transform->layer);
    glm::vec4 light_clip_pos = projection * view * glm::vec4(light_world_pos, 1.0f);
    glm::vec2 light_texcoord = ((glm::vec2(light_clip_pos) / light_clip_pos.w) + 1.0f) * 0.5f;
    // TODO: Pack data into vec4s, reduce calls by half.
		shader->SetUniform("lights[" + std::to_string(i) + "].type", static_cast<int>(light->type));
    shader->SetUniform("lights[" + std::to_string(i) + "].position", light_texcoord);
    shader->SetUniform("lights[" + std::to_string(i) + "].color", light->color);
    shader->SetUniform("lights[" + std::to_string(i) + "].intensity", light->intensity);
    shader->SetUniform("lights[" + std::to_string(i) + "].radial_falloff", light->radial_falloff);
		shader->SetUniform("lights[" + std::to_string(i) + "].volumetric_intensity", light->volumetric_intensity);
  }
  BindData(quad);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  UnbindData();
  return 0;
}
int ion::render::Quit() {
  for (auto& [framebuffer, name] : internal::framebuffers) {
    glDeleteFramebuffers(1, &framebuffer->framebuffer);
    glDeleteTextures(1, &framebuffer->colorbuffer);
  }
	internal::framebuffers.clear();  
  glfwDestroyWindow(internal::window);
  glfwTerminate();
  return 0;
}
void ion::render::Present() {
  glfwSwapBuffers(internal::window);
}