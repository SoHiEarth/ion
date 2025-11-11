#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"
#include "error_code.h"
#include <glad/glad.h>
#include <iostream>
#include <stb_image.h>

void Texture::Load(std::string_view path) {
  int width, height, nr_channels;
  unsigned char *data =
      stbi_load(path.data(), &width, &height, &nr_channels, 0);
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cerr << TEXTURE_LOAD_FAIL << std::endl;
  }
  stbi_image_free(data);
}
