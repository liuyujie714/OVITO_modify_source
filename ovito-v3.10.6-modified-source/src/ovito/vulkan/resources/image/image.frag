#version 440

// Inputs:
layout(location = 0) in vec2 texcoord_fs;

// Outputs:
layout(location = 0) out vec4 fragColor;

// Texture sampler:
layout(set = 0, binding = 0) uniform sampler2D tex1;

void main()
{
    fragColor = texture(tex1, texcoord_fs);
}
