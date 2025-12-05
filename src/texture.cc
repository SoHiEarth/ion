#include "texture.h"
#include <glad/glad.h>

void Texture::Use() {
  glBindTexture(GL_TEXTURE_2D, texture);
}

void TexturePack::Use() {
  if (color) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color->texture);
  }
  if (normal) {
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normal->texture);
  }
}