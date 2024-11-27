#version 440

// Push constants:
layout(push_constant) uniform constants {
    mat4 mvp;
    // vec4 color; -> used in the fragment shader
} PushConstants;

// Inputs:
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 instance_tm_row1;
layout(location = 2) in vec4 instance_tm_row2;
layout(location = 3) in vec4 instance_tm_row3;

// Outputs:
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
}
