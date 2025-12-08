#pragma once
#include <filesystem>
#include <string>

struct Shader {
private:
  unsigned int program;
  std::string id;
  std::filesystem::path path;

public:
  void Use();
  std::string GetID() const { return id; }
  std::filesystem::path GetPath() const { return path; }
  unsigned int GetProgram();
  template <typename T> int SetUniform(std::string_view name, T value);
  explicit Shader(std::filesystem::path path, std::string_view new_id);
};
