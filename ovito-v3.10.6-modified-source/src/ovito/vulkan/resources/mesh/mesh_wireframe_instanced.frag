#version 440

// Push constants:
layout(push_constant) uniform constants_fragment {
    // mat4 mvp; -> used in the vertex shader
    layout(offset = 64) vec4 color;
} PushConstants;

// Outputs:
layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = PushConstants.color;
}
