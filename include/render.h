#pragma once
#include "component.h"
#include "gpu_data.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include "exports.h"

struct Transform;
struct Texture;
struct Shader;
struct World;
enum class DataType : std::uint8_t { INT, UNSIGNED_INT, FLOAT };
struct ION_API AttributePointer {
  int size = 2;
  DataType type = DataType::FLOAT;
  bool normalized = false;
  size_t stride = 0;
  const void *pointer = 0;
};

struct ION_API DataDescriptor {
  std::vector<AttributePointer> pointers;
  bool element_enabled = false;
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
};

struct ION_API TextureInfo {
  unsigned char *data;
  int width;
  int height;
  int nr_channels;
};

struct ION_API FramebufferInfo {
  bool enable_colorbuffer = true;
  bool recreate_on_resize = false;
  std::string name = "NO_LABEL";
};

struct ION_API Framebuffer {
  bool recreate_on_resize = false;
  unsigned int framebuffer = 0;
  unsigned int colorbuffer = 0;
};

enum RenderPass {
  RENDER_PASS_COLOR,
  RENDER_PASS_NORMAL
};

class ION_API RenderSystem {
private:
  GLFWwindow *window;
  std::map<std::shared_ptr<Framebuffer>, std::string> framebuffers;

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
	const std::map<std::shared_ptr<Framebuffer>, std::string>& GetFramebuffers() const { return framebuffers; }

	void BindTexture(std::shared_ptr<Texture> texture, int slot);
  void BindTexture(std::shared_ptr<Framebuffer> framebuffer, int slot);

  void UseShader(std::shared_ptr<Shader> shader);
	void RunPass(std::shared_ptr<Framebuffer> in, std::shared_ptr<Framebuffer> out, std::shared_ptr<Shader> shader, std::shared_ptr<GPUData> quad);

  void DrawWorld(std::shared_ptr<World>, RenderPass);
  void DestroyShader(std::shared_ptr<Shader>);
  void Clear(glm::vec4);
  int Render(std::shared_ptr<Framebuffer> color, std::shared_ptr<Framebuffer> normal, std::shared_ptr<GPUData> data, std::shared_ptr<Shader> shader, std::shared_ptr<World> world);
  void Present();
  int Quit();
};
