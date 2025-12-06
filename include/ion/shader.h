#pragma once
#include <string>
#include <filesystem>

struct Shader {
private:
  unsigned int program;
  std::string id;
public:
  void Use();
	std::string GetID() const { return id; }
  unsigned int GetProgram();
  template <typename T> int SetUniform(std::string_view name, T value);
  explicit Shader(std::filesystem::path path, std::string_view new_id);
};
