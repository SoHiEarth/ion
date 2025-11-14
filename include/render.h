#pragma once
#include "transform.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
class Texture;
class Shader;

class RenderSystem {
  GLFWwindow *window;

public:
  int Init();
  GLFWwindow *GetWindow();
  static void RenderSprite(Transform transform, GetModelFlags model_flags,
                           Texture texture, Shader shader);
  int Render();
  int Quit();
};
