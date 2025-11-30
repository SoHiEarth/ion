#version 330 core

in vec2 TexCoords;
uniform sampler2D ION_PASS_IN;
uniform sampler2D ION_PASS_FRAMEBUFFER;
out vec4 FragColor;

void main() {
  vec4 sampled = texture(ION_PASS_IN, TexCoords);
  vec4 sampled_framebuffer = texture(ION_PASS_FRAMEBUFFER, TexCoords);
  FragColor = sampled + sampled_framebuffer;
}