#version 330 core

in vec2 TexCoords;
uniform sampler2D ION_PASS_IN;
out vec4 FragColor;

void main() {             
    const float gamma = 2.2;
    vec3 color = texture(ION_PASS_IN, TexCoords).rgb;
    vec3 mapped = color / (color + vec3(1.0));
    mapped = pow(mapped, vec3(1.0 / gamma));  
    FragColor = vec4(mapped, 1.0);
}    