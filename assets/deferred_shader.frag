#version 330 core

struct Light {
  vec2 position;
  float intensity;
  float radial_falloff;
  vec3 color;
};

const int MAX_LIGHTS = 32;

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D color_texture;
uniform sampler2D normal_texture;
uniform int light_count;
uniform Light lights[MAX_LIGHTS];

void main() {
  vec4 sample = texture(color_texture, TexCoord);
  if (sample.a == 0.0) {
    discard;
  }
  vec3 color = sample.rgb;
  vec3 normal = texture(normal_texture, TexCoord).rgb * 2.0 - 1.0;
  normal = normalize(normal);
  
  vec3 total_lighting = vec3(0.0);  
  for (int i = 0; i < light_count && i < MAX_LIGHTS; i++) {
    vec3 light_dir = normalize(vec3(lights[i].position - (TexCoord * 2.0 - 1.0), 0.0));
    float distance = length(vec3(lights[i].position - (TexCoord * 2.0 - 1.0), 0.0));
    float attenuation = lights[i].intensity / (1.0 + lights[i].radial_falloff * distance * distance);
    float diff = max(dot(normal, light_dir), 0.0);
    total_lighting += lights[i].color * diff * attenuation;
  }
  
  FragColor = vec4(color * total_lighting, 1.0);
}