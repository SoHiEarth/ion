#pragma once
#include <string_view>

struct Shader {
private:
  unsigned int program;

public:
  void Use();
  unsigned int GetProgram();
  template <typename T> int SetUniform(std::string_view name, T value);
  explicit Shader(std::string_view vertex_path, std::string_view fragment_path);
};
