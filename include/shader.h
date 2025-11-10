#pragma once
#include <string_view>

class Shader {
private:
  unsigned int program;

public:
  void Use();
  unsigned int GetProgram();
  explicit Shader(std::string_view vertex_path, std::string_view fragment_path);
};
