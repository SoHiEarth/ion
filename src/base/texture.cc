#include "ion/texture.h"
#include <glad/glad.h>

void Texture::Use() { glBindTexture(GL_TEXTURE_2D, texture); }