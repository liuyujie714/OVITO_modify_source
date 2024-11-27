#version 440

#include "../picking.glsl"

// Push constants:
layout(push_constant) uniform constants {
    mat4 mvp;
    int pickingBaseId;
} PushConstants;

// Inputs:
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 instance_tm_row1;
layout(location = 2) in vec4 instance_tm_row2;
layout(location = 3) in vec4 instance_tm_row3;

// Outputs:
layout(location = 0) out vec4 color_fs;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
    // Apply instance transformation.
    vec3 instance_position = vec3(
        dot(instance_tm_row1, position), 
        dot(instance_tm_row2, position), 
        dot(instance_tm_row3, position));

	// Apply model-view-projection matrix to vertex.
    gl_Position = PushConstants.mvp * vec4(instance_position, 1.0);

    // Compute color from object ID.
    color_fs = pickingModeColor(PushConstants.pickingBaseId, gl_InstanceIndex);
}
