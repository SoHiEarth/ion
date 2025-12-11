#include "ion/render.h"
#include "ion/component.h"
#include "ion/error_code.h"
#include "ion/render/api.h"
#include "ion/shader.h"
#include "ion/texture.h"
#include "ion/world.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <stb_image.h>
#include <string>
#include <vulkan/vulkan.h>

namespace ion::render::internal {
ION_API GLFWwindow *window = nullptr;
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

static void SizeCallback(GLFWwindow *window, int w, int h) {
  r_config.window_size.x = w;
  r_config.window_size.y = h;
  ion::render::UpdateFramebuffers();
}
static glm::mat4 GetModelFromTransform(std::shared_ptr<Transform> transform) {
  auto model = glm::mat4(1.0f);
  model =
      glm::translate(model, glm::vec3(transform->position.x,
                                      transform->position.y, transform->layer));
  model = glm::rotate(model, glm::radians(transform->rotation),
                      glm::vec3(0.0f, 0.0f, 1.0f));
  model =
      glm::scale(model, glm::vec3(transform->scale.x, transform->scale.y, 1));
  return model;
}

int ion::render::Init() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  internal::window = glfwCreateWindow(
      r_config.window_size.x, r_config.window_size.y, "ion", NULL, NULL);
  if (!internal::window) {
    printf("%d\n", WINDOW_CREATE_FAIL);
    glfwTerminate();
    return -1;
  }
  api::CreateInstance();
  api::CreateMessenger();
  api::CreateSurface();
  api::CreateDevice();
  api::CreateSwapchain();
  api::CreateImageViews();

  glfwSetFramebufferSizeCallback(internal::window, SizeCallback);
  return 0;
}
GLFWwindow *ion::render::GetWindow() { return internal::window; }
glm::vec2 ion::render::GetWindowSize() { return r_config.window_size; }
int ion::render::GetRenderScale() { return r_config.render_scale; }
void ion::render::SetRenderScale(int scale) {
  r_config.render_scale = scale;
  UpdateFramebuffers();
}
glm::vec3 ion::render::GetClearColor() { return r_config.clear_color; }
void ion::render::SetClearColor(glm::vec3 color) {
  r_config.clear_color = color;
}

void ion::render::ConfigureData(std::shared_ptr<GPUData> gpu_data) {
  auto desc = gpu_data->GetDescriptor();
  gpu_data->element_enabled = desc.element_enabled;
  if (gpu_data->element_enabled) {
  }
  if (gpu_data->element_enabled) {
  }
  for (int i = 0; i < desc.pointers.size(); i++) {
    auto &pointer_data = desc.pointers[i];
  }
  UnbindData();
}
void ion::render::DestroyData(std::shared_ptr<GPUData> data) {
  if (data->element_enabled) {
  }
}
void ion::render::BindData(std::shared_ptr<GPUData> data) {
  if (data->element_enabled) {
  }
}
void ion::render::UnbindData() {}

unsigned int ion::render::ConfigureTexture(const TextureInfo &texture_info) {
  unsigned int texture = 0;
  if (texture_info.data) {
  } else {
    printf("%d\n", TEXTURE_LOAD_FAIL);
  }
  return texture;
}

std::shared_ptr<Framebuffer>
ion::render::CreateFramebuffer(const FramebufferInfo &info) {
  auto framebuffer = std::make_shared<Framebuffer>();
  framebuffer->recreate_on_resize = info.recreate_on_resize;
  internal::framebuffers.insert({framebuffer, info.name});
  return framebuffer;
}
void ion::render::UpdateFramebuffers() {
  for (auto &[framebuffer, name] : internal::framebuffers) {
    if (framebuffer->recreate_on_resize) {
    }
  }
}
void ion::render::BindFramebuffer(std::shared_ptr<Framebuffer> framebuffer) {}
void ion::render::UnbindFramebuffer() {}

void ion::render::DrawFramebuffer(std::shared_ptr<Framebuffer> framebuffer,
                                  std::shared_ptr<Shader> shader,
                                  std::shared_ptr<GPUData> quad,
                                  std::shared_ptr<Framebuffer> final_buffer) {
  if (final_buffer) {
  } else {
  }
  shader->Use();
  BindData(quad);
  shader->SetUniform("screen_texture", 0);
  if (quad->element_enabled) {
  } else {
  }
  UnbindData();
}
const std::map<std::shared_ptr<Framebuffer>, std::string> &
ion::render::GetFramebuffers() {
  return internal::framebuffers;
}

void ion::render::BindTexture(std::shared_ptr<Texture> texture, int slot) {}
void ion::render::BindTexture(std::shared_ptr<Framebuffer> framebuffer,
                              int slot) {}

void ion::render::UseShader(std::shared_ptr<Shader> shader) {}
void ion::render::DestroyShader(std::shared_ptr<Shader> shader) {
  shader.reset();
}

void ion::render::DrawWorld(std::shared_ptr<World> world, RenderPass pass) {
  for (auto &[entity_id, camera] : world->GetComponentSet<Camera>()) {
    glm::mat4 view =
        GetModelFromTransform(world->GetComponent<Transform>(entity_id));
    view = glm::translate(view, glm::vec3{0.0, 0.0, -3.0});
    float ortho_scale = 10.0f;
    auto projection = glm::ortho(
        -ortho_scale *
            (ion::render::GetWindowSize().x / ion::render::GetWindowSize().y),
        ortho_scale *
            (ion::render::GetWindowSize().x / ion::render::GetWindowSize().y),
        -ortho_scale, ortho_scale, 0.1f, 100.0f);
    for (auto &[entity_id, transform] : world->GetComponentSet<Transform>()) {
      auto renderable = world->GetComponent<Renderable>(entity_id);
      if (renderable) {
        if (!renderable->shader || !renderable->data || !renderable->color ||
            !renderable->normal) {
          continue;
        }
        BindData(renderable->data);
        renderable->shader->Use();
        renderable->shader->SetUniform("layer", transform->layer);
        renderable->shader->SetUniform("view", view);
        renderable->shader->SetUniform("projection", projection);
        renderable->shader->SetUniform("model",
                                       GetModelFromTransform(transform));
        if (pass == RENDER_PASS_COLOR) {
        } else if (pass == RENDER_PASS_NORMAL) {
        }
        renderable->shader->SetUniform("sample", 0);
        UnbindData();
      }
    }
  }
}
void ion::render::RunPass(std::shared_ptr<Framebuffer> in,
                          std::shared_ptr<Framebuffer> out,
                          std::shared_ptr<Shader> shader,
                          std::shared_ptr<GPUData> quad) {
  BindFramebuffer(out);
  shader->SetUniform("ION_PASS_IN", 0);
  BindTexture(in, 0);
  BindData(quad);
  if (quad->element_enabled) {
  } else {
  }
  UnbindData();
}

void ion::render::Clear() {}
void ion::render::Clear(glm::vec4 color) {}
int ion::render::Render(std::shared_ptr<Framebuffer> color_fb,
                        std::shared_ptr<Framebuffer> normal_fb,
                        std::shared_ptr<GPUData> quad,
                        std::shared_ptr<Shader> shader,
                        std::shared_ptr<World> world) {
  shader->Use();
  shader->SetUniform("color_texture", 0);
  shader->SetUniform("normal_texture", 1);
  shader->SetUniform("light_count",
                     static_cast<int>(world->GetComponentSet<Light>().size()));

  auto view = glm::mat4(1.0f);
  float ortho_scale = 10.0f;
  auto projection =
      glm::ortho(-ortho_scale * (GetWindowSize().x / GetWindowSize().y),
                 ortho_scale * (GetWindowSize().x / GetWindowSize().y),
                 -ortho_scale, ortho_scale, 0.1f, 100.0f);
  for (auto &[id, camera] : world->GetComponentSet<Camera>()) {
    view = GetModelFromTransform(world->GetComponent<Transform>(id));
    view = glm::translate(view, glm::vec3(0.0, 0.0, -3.0));
  }

  for (int i = 0; i < world->GetComponentSet<Light>().size(); i++) {
    auto &[entity_id, light] =
        *std::next(world->GetComponentSet<Light>().begin(), i);
    auto transform = world->GetComponent<Transform>(entity_id);
    glm::vec3 light_world_pos =
        glm::vec3(transform->position, transform->layer);
    glm::vec4 light_clip_pos =
        projection * view * glm::vec4(light_world_pos, 1.0f);
    glm::vec2 light_texcoord =
        ((glm::vec2(light_clip_pos) / light_clip_pos.w) + 1.0f) * 0.5f;
    // TODO: Pack data into vec4s, reduce calls by half.
    shader->SetUniform("lights[" + std::to_string(i) + "].type",
                       static_cast<int>(light->type));
    shader->SetUniform("lights[" + std::to_string(i) + "].position",
                       light_texcoord);
    shader->SetUniform("lights[" + std::to_string(i) + "].color", light->color);
    shader->SetUniform("lights[" + std::to_string(i) + "].intensity",
                       light->intensity);
    shader->SetUniform("lights[" + std::to_string(i) + "].radial_falloff",
                       light->radial_falloff);
    shader->SetUniform("lights[" + std::to_string(i) + "].volumetric_intensity",
                       light->volumetric_intensity);
  }
  BindData(quad);
  UnbindData();
  return 0;
}
int ion::render::Quit() {
  for (auto &[framebuffer, name] : internal::framebuffers) {
  }
  internal::framebuffers.clear();
  api::Quit();
  glfwDestroyWindow(internal::window);
  glfwTerminate();
  return 0;
}
void ion::render::Present() {}
