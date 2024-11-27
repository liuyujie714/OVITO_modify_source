#version 440

// Push constants:
layout(push_constant) uniform constants {
    mat4 mvp;
} PushConstants;

// Inputs:
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

// Outputs:
layout(location = 0) out vec4 color_fs;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
    // Forward vertex color to fragment shader.
    color_fs = color;

	// Apply model-view-projection matrix.
    gl_Position = PushConstants.mvp * position;
}
