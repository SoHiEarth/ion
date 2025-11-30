#version 330 core

in vec2 TexCoords;
uniform sampler2D ION_PASS_IN;
out vec4 FragColor;

void main() {
  vec4 sampled = texture(ION_PASS_IN, TexCoords);
  float brightness = dot(sampled.rgb, vec3(0.2126, 0.7152, 0.0722));
  if (brightness > 1.0)
    FragColor = vec4(sampled.rgb, 1.0);
  else
    FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
