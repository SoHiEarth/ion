#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D sample;

void main() {
  vec4 sampled = texture(sample, TexCoord);
  if (sampled.a == 0.0) {
    discard;
  }
  FragColor = sampled;
}
