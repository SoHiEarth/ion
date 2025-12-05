#version 330 core

in vec2 TexCoords;
uniform sampler2D ION_PASS_IN;
uniform sampler2D ION_PASS_FRAMEBUFFER;
uniform float bloom_strength = 0.5;
out vec4 FragColor;

void main() {
  vec4 bloom = texture(ION_PASS_IN, TexCoords);
  vec4 original = texture(ION_PASS_FRAMEBUFFER, TexCoords);
  FragColor = original + bloom * bloom_strength;
}