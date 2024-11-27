#version 440

#include "../global_uniforms.glsl"
#include "../shading.glsl"

// Inputs:
layout(location = 0) flat in vec4 color_fs;
layout(location = 1) in vec3 normal_fs;

// Outputs:
layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = shadeSurfaceColor(normalize(normal_fs), color_fs);
}
