#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
out vec3 FragPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
  TexCoord = aTexCoord;
  vec4 worldPos = model * vec4(aPos, 0.0, 1.0);
  FragPos = worldPos.xyz;
  gl_Position = projection * view * worldPos;
}
