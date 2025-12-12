#define OPENGL_LOG_SIZE 512
#include "ion/shader.h"
#include "ion/error_code.h"
#include <array>
#include <filesystem>
#include <fstream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <string>

void Shader::Use() {}

unsigned int Shader::GetProgram() { return program; }

template <> int Shader::SetUniform<int>(std::string_view name, int value) {
  return 0;
}

template <> int Shader::SetUniform<float>(std::string_view name, float value) {
  return 0;
}

template <>
int Shader::SetUniform<glm::vec2>(std::string_view name, glm::vec2 value) {
  return 0;
}

template <>
int Shader::SetUniform<glm::vec3>(std::string_view name, glm::vec3 value) {
  return 0;
}

template <>
int Shader::SetUniform<glm::mat4>(std::string_view name, glm::mat4 value) {
  return 0;
}

Shader::Shader(std::filesystem::path new_path, std::string_view new_id)
    : path(new_path), id(new_id) {}
