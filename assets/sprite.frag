#version 330 core

struct Light {
  vec2 position;
  float intensity;
  float radial_falloff;
  float angular_falloff;
  vec3 color;
};

const int MAX_LIGHTS = 32;

in vec3 FragPos;
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D color;
uniform int light_count;
uniform Light lights[MAX_LIGHTS];

void main() {
  vec4 base_sample = texture(color, TexCoord);
  if (base_sample.a == 0.0) {
    discard;
  }

  vec3 base = base_sample.rgb;
  vec3 lighting = vec3(0.0);
  
  for (int i = 0; i < light_count && i < MAX_LIGHTS; i++) {
    float distance = length(vec3(lights[i].position, 0.0) - FragPos);
    float attenuation = 1.0 / (1.0 + lights[i].radial_falloff * distance * distance);
    lighting += lights[i].color * lights[i].intensity * attenuation;
  }
  
  FragColor = vec4(base * lighting, base_sample.a);
}
