#pragma once
#include "component.h"
#include "gpu_data.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

struct Transform;
struct Texture;
struct Shader;
struct World;
enum class DataType : std::uint8_t { INT, UNSIGNED_INT, FLOAT };
struct AttributePointer {
  int size = 2;
  DataType type = DataType::FLOAT;
  bool normalized = false;
  size_t stride = 0;
  const void *pointer;
};

struct DataDescriptor {
  std::vector<AttributePointer> pointers;
  bool element_enabled = false;
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
};

struct TextureInfo {
  unsigned char *data;
  int width;
  int height;
  int nr_channels;
};

struct FramebufferInfo {
  bool enable_colorbuffer = true;
  bool recreate_on_resize = false;
};

struct Framebuffer {
  bool recreate_on_resize = false;
  unsigned int framebuffer = 0;
  unsigned int colorbuffer = 0;
};

enum RenderPass {
  RENDER_PASS_COLOR,
  RENDER_PASS_NORMAL
};

class RenderSystem {
private:
  GLFWwindow *window;
  std::vector<std::shared_ptr<Framebuffer>> framebuffers;

public:
  int Init();
  GLFWwindow *GetWindow();
  glm::vec2 GetWindowSize();
  std::shared_ptr<GPUData> CreateData(DataDescriptor&);
  void DestroyData(std::shared_ptr<GPUData>);
  void BindData(std::shared_ptr<GPUData>);
  void UnbindData();

  unsigned int ConfigureTexture(const TextureInfo& texture_info);
  
  std::shared_ptr<Framebuffer> CreateFramebuffer(FramebufferInfo&);
  void UpdateFramebuffers();
  void BindFramebuffer(std::shared_ptr<Framebuffer>);
  void UnbindFramebuffer();
  void DrawFramebuffer(std::shared_ptr<Framebuffer>, std::shared_ptr<Shader>, std::shared_ptr<GPUData> quad);

  void DrawWorld(std::shared_ptr<World>, RenderPass);
  void DestroyShader(std::shared_ptr<Shader>);
  void Clear(glm::vec4);
  int Render(std::shared_ptr<Framebuffer> color_framebuffer, std::shared_ptr<Framebuffer> normal_framebuffer, std::shared_ptr<GPUData> data, std::shared_ptr<Shader> shader, std::shared_ptr<World> world);
  void Present();
  int Quit();
};
