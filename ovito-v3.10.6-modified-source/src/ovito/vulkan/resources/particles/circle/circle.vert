#version 440

// Push constants:
layout(push_constant) uniform constants {
    mat4 projection_matrix;
    layout(row_major) mat4x3 modelview_matrix;
} PushConstants;

// Inputs:
layout(location = 0) in vec3 position;
layout(location = 1) in float radius;
layout(location = 2) in vec4 color;

// Outputs:
layout(location = 0) out vec4 color_fs;
layout(location = 1) out vec2 uv_fs;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
	// Const array of vertex positions for the quad triangle strip.
	const vec2 quad[4] = vec2[4](
        vec2(-1.0, -1.0),
        vec2( 1.0, -1.0),
        vec2(-1.0,  1.0),
        vec2( 1.0,  1.0)
	);

    // The index of the quad corner.
    int corner = gl_VertexIndex;

    // Transform particle center to view space.
	vec3 eye_position = PushConstants.modelview_matrix * vec4(position, 1.0);

    // Apply additional scaling due to model-view transformation to particle radius. 
    float viewspace_radius = radius * length(PushConstants.modelview_matrix[0]);

	// Project corner vertex.
    gl_Position = PushConstants.projection_matrix * (vec4(eye_position, 1.0) + vec4(quad[corner] * viewspace_radius, 0.0, 0.0));

    // Forward particle color to fragment shader.
    color_fs = color;

    // Pass UV quad coordinates to fragment shader.
    uv_fs = quad[corner];
}
