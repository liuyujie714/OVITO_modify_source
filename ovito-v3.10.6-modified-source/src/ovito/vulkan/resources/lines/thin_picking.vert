#version 440

#include "../picking.glsl"

// Push constants:
layout(push_constant) uniform constants {
    mat4 mvp;
    int pickingBaseId;
} PushConstants;

// Inputs:
layout(location = 0) in vec4 position;

// Outputs:
layout(location = 0) out vec4 color_fs;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
    // Compute color from object ID.
    color_fs = pickingModeColor(PushConstants.pickingBaseId, gl_VertexIndex / 2);

	// Apply model-view-projection matrix.
    gl_Position = PushConstants.mvp * position;
}
