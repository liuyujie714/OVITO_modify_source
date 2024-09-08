#version 440

// Push constants:
layout(push_constant) uniform constants {
    mat4 mvp;
    mat4 normal_tm;
} PushConstants;

// Inputs:
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;

// Outputs:
layout(location = 0) out vec4 color_fs;
layout(location = 1) out vec3 normal_fs;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
	// Apply model-view-projection matrix to vertex.
    gl_Position = PushConstants.mvp * vec4(position, 1.0);

    // Pass vertex color on to fragment shader.
    color_fs = color;

    // Transform vertex normal from object to view space.
    normal_fs = vec3(PushConstants.normal_tm * vec4(normal, 0.0));
}
