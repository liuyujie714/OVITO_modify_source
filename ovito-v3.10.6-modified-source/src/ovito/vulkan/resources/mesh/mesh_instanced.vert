#version 440

// Push constants:
layout(push_constant) uniform constants {
    mat4 mvp;
    mat4 normal_tm;
} PushConstants;

// Inputs:
layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec4 instance_tm_row1;
layout(location = 4) in vec4 instance_tm_row2;
layout(location = 5) in vec4 instance_tm_row3;

// Outputs:
layout(location = 0) out vec4 color_fs;
layout(location = 1) out vec3 normal_fs;
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

    // Pass vertex color on to fragment shader.
    color_fs = color;

    // Apply instance transformation to normal vector.
    // Note: We are assuming a pure rotation here.
    vec3 instance_normal = vec3(
        dot(instance_tm_row1, vec4(normal, 0.0)), 
        dot(instance_tm_row2, vec4(normal, 0.0)), 
        dot(instance_tm_row3, vec4(normal, 0.0)));

    // Transform vertex normal from object to view space.
    normal_fs = vec3(PushConstants.normal_tm * vec4(instance_normal, 0.0));
}
