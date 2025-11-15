#pragma once
#include "transform.h"
#include <glm/glm.hpp>
#include <vector>

class Texture;
class Shader;
struct GLFWwindow;
enum class DataType { INT, UNSIGNED_INT, FLOAT };
enum class RenderMode { FILL, LINE };
struct AttributePointer {
  int size = 3;
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

struct GPUData {
  unsigned int vertex_attrib, vertex_buffer;
  bool element_enabled = false;
  unsigned int element_buffer;
};

class RenderSystem {
private:
  GLFWwindow *window;

public:
  int Init();
  GLFWwindow *GetWindow();
  glm::vec2 GetWindowSize();
  void SetMode(RenderMode mode);
  GPUData CreateData(DataDescriptor attribute_data);
  void DestroyData(GPUData data);
  void BindData(GPUData data);
  void UnbindData();
  static void RenderSprite(Transform transform, GetModelFlags model_flags,
                           Texture texture, Shader shader);

  void DestroyShader(Shader &shader);
  void Clear(glm::vec4 color);
  int Render();
  int Quit();
};
