#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D screen_texture;
void main() {
  FragColor = texture(screen_texture, TexCoord);
}