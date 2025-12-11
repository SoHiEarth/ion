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

std::string _ShaderInternalReadFile(std::filesystem::path path) {
  std::string data{};
  std::ifstream file{};
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    file.open(path);
    std::stringstream stream;
    stream << file.rdbuf();
    file.close();
    data = stream.str();
  } catch (std::ifstream::failure e) {
    printf("File read fail: %s, %d\n", path.string().c_str(), FILE_READ_FAIL);
  }
  return data;
}

Shader::Shader(std::filesystem::path new_path, std::string_view new_id)
    : path(new_path), id(new_id) {
  unsigned int vertex, fragment;
  int success;
  std::array<char, OPENGL_LOG_SIZE> info_log = {};
  auto vertex_code = _ShaderInternalReadFile(path / "vs.glsl"),
       fragment_code = _ShaderInternalReadFile(path / "fs.glsl");
  auto vertex_code_char = vertex_code.c_str(),
       fragment_code_char = fragment_code.c_str();
  if (!success) {
    printf("Error while compiling vertex shader %s\n",
           (path / "vs.glsl").c_str());
    printf("%s\n", info_log.data());
    printf("%d\n", VERTEX_COMPILATION_FAIL);
  }
  if (!success) {
    printf("Error while compiling fragment shader %s\n",
           (path / "fs.glsl").c_str());
    printf("%s\n", info_log.data());
    printf("%d\n", FRAGMENT_COMPILATION_FAIL);
  }
  if (!success) {
    printf("%s\n", info_log.data());
    printf("%d\n", SHADER_PROGRAM_LINK_FAIL);
  }
}
