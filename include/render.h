#pragma once
#include <GLFW/glfw3.h>
#include "component.h"
#include "gpu_data.h"
#include <glm/glm.hpp>
#include <vector>

struct Transform;
struct Texture;
struct Shader;
struct World;
enum class DataType { INT, UNSIGNED_INT, FLOAT };
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
  unsigned char* data;
  int width;
  int height;
  int nr_channels;
};

class RenderSystem {
private:
  GLFWwindow *window;

public:
  int Init();
  GLFWwindow *GetWindow();
  glm::vec2 GetWindowSize();
  std::shared_ptr<GPUData> CreateData(DataDescriptor& attribute_data);
  void DestroyData(std::shared_ptr<GPUData> data);
  void BindData(std::shared_ptr<GPUData> data);
  void UnbindData();

  unsigned int ConfigureTexture(TextureInfo texture_info);

  void DrawWorld(World& world, RenderSystem& render_sys);
  void DestroyShader(std::shared_ptr<Shader> shader);
  void Clear(glm::vec4 color);
  int Render();
  int Quit();
};
