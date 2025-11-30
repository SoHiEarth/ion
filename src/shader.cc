#define OPENGL_LOG_SIZE 512
#include "shader.h"
#include "error_code.h"
#include <array>
#include <fstream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <string>

void Shader::Use() { glUseProgram(program); }
unsigned int Shader::GetProgram() { return program; }

template <> int Shader::SetUniform<int>(std::string_view name, int value) {
  auto loc = glGetUniformLocation(program, name.data());
  glUniform1i(loc, value);
  return 0;
}

template <>
int Shader::SetUniform<float>(std::string_view name, float value) {
  auto loc = glGetUniformLocation(program, name.data());
  glUniform1f(loc, value);
  return 0;
}

template <>
int Shader::SetUniform<glm::vec2>(std::string_view name, glm::vec2 value) {
  auto loc = glGetUniformLocation(program, name.data());
  glUniform2fv(loc, 1, glm::value_ptr(value));
  return 0;
}

template <>
int Shader::SetUniform<glm::vec3>(std::string_view name, glm::vec3 value) {
  auto loc = glGetUniformLocation(program, name.data());
  glUniform3fv(loc, 1, glm::value_ptr(value));
  return 0;
}

template <>
int Shader::SetUniform<glm::mat4>(std::string_view name, glm::mat4 value) {
  auto loc = glGetUniformLocation(program, name.data());
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
  return 0;
}

std::string _ShaderInternalReadFile(std::string_view path) {
  std::string data{};
  std::ifstream file{};
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    file.open(path.data());
    std::stringstream stream;
    stream << file.rdbuf();
    file.close();
    data = stream.str();
  } catch (std::ifstream::failure e) {
    printf("%d\n", FILE_READ_FAIL);
  }
  return data;
}

Shader::Shader(std::string_view vertex_path, std::string_view fragment_path) {
  unsigned int vertex, fragment;
  int success;
  std::array<char, OPENGL_LOG_SIZE> info_log;
  vertex = glCreateShader(GL_VERTEX_SHADER);
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  auto vertex_code = _ShaderInternalReadFile(vertex_path),
       fragment_code = _ShaderInternalReadFile(fragment_path);
  auto vertex_code_char = vertex_code.c_str(),
       fragment_code_char = fragment_code.c_str();
  glShaderSource(vertex, 1, &vertex_code_char, nullptr);
  glCompileShader(vertex);
  glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex, info_log.size(), nullptr, info_log.data());
    printf("Error while compiling vertex shader %s\n", vertex_path.data());
    printf("%s\n", info_log.data());
    printf("%d\n", VERTEX_COMPILATION_FAIL);
  }
  glShaderSource(fragment, 1, &fragment_code_char, nullptr);
  glCompileShader(fragment);
  glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment, 512, nullptr, info_log.data());
		printf("Error while compiling fragment shader %s\n", fragment_path.data());
    printf("%s\n", info_log.data());
    printf("%d\n", FRAGMENT_COMPILATION_FAIL);
  }
  program = glCreateProgram();
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, nullptr, info_log.data());
    printf("%s\n", info_log.data());
    printf("%d\n", SHADER_PROGRAM_LINK_FAIL);
  }
  glDeleteShader(vertex);
  glDeleteShader(fragment);
}
