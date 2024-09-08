#version 440

// Push constants:
layout(push_constant) uniform constants {
    mat4 mvp;
    mat4 normal_tm;
} PushConstants;

// Inputs:
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 pseudocolor;

// Outputs:
layout(location = 0) out vec2 pseudocolor_fs;
layout(location = 1) out vec3 normal_fs;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
	// Apply model-view-projection matrix to vertex.
    gl_Position = PushConstants.mvp * vec4(position, 1.0);

    // Pseudo-color value range is stored in unused elements of the normal transformation matrix.
    // That's because the Vulkan push constants are limited to 128 bytes (fitting exactly two mat4 structures).
    float color_range_min = PushConstants.normal_tm[3][0];
    float color_range_max = PushConstants.normal_tm[3][1];

    // Pass normalized pseudo-color information on to fragment shader.
    pseudocolor_fs.x = (pseudocolor.r - color_range_min) / (color_range_max - color_range_min);
    pseudocolor_fs.y = (pseudocolor.g == 0.0) ? pseudocolor.a : -1.0; // Note: A non-zero color component G indicates selected faces.

    // Transform vertex normal from object to view space.
    normal_fs = vec3(PushConstants.normal_tm * vec4(normal, 0.0));
}
