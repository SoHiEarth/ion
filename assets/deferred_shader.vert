#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 FragPos;

void main() {
  TexCoord = aTexCoord;
  FragPos = vec3(aPos, 0.0);
  gl_Position = vec4(aPos, 0.0, 1.0);
}