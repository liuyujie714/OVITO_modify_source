#version 440

// Inputs:
layout(location = 0) flat in vec4 color_fs;

// Outputs:
layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = color_fs;
}
