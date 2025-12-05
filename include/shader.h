#pragma once
#include <string_view>
#include <filesystem>

struct Shader {
private:
  unsigned int program;

public:
  void Use();
  unsigned int GetProgram();
  template <typename T> int SetUniform(std::string_view name, T value);
  explicit Shader(std::filesystem::path vertex_path, std::filesystem::path fragment_path);
};
