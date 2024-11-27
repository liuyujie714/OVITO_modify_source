#version 440

#include "../../global_uniforms.glsl"
#include "../../picking.glsl"

// Push constants:
layout(push_constant) uniform constants {
    mat4 mvp;
    layout(row_major) mat4x3 modelview_matrix;
    int pickingBaseId;
} PushConstants;

// Inputs:
layout(location = 0) in vec3 position;
layout(location = 1) in float radius;

// Outputs:
layout(location = 0) out vec4 color_fs;
layout(location = 1) out vec3 particle_view_pos_fs;
layout(location = 2) out float particle_radius_squared_fs;
layout(location = 3) out vec3 ray_origin;
layout(location = 4) out vec3 ray_dir;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
	// Const array of vertex positions for the cube triangle strip.
	const vec3 cube[14] = vec3[14](
        vec3( 1.0,  1.0,  1.0),
        vec3( 1.0, -1.0,  1.0),
        vec3( 1.0,  1.0, -1.0),
        vec3( 1.0, -1.0, -1.0),
        vec3(-1.0, -1.0, -1.0),
        vec3( 1.0, -1.0,  1.0),
        vec3(-1.0, -1.0,  1.0),
        vec3( 1.0,  1.0,  1.0),
        vec3(-1.0,  1.0,  1.0),
        vec3( 1.0,  1.0, -1.0),
        vec3(-1.0,  1.0, -1.0),
        vec3(-1.0, -1.0, -1.0),
        vec3(-1.0,  1.0,  1.0),
        vec3(-1.0, -1.0,  1.0)
	);

    // The index of the cube corner.
    int corner = gl_VertexIndex;

	// Apply model-view-projection matrix to particle position displaced by the cube vertex position.
    gl_Position = PushConstants.mvp * vec4(position + cube[corner] * radius, 1.0);

    // Compute color from object ID.
    color_fs = pickingModeColor(PushConstants.pickingBaseId, gl_InstanceIndex);

    // Pass particle radius and center position to fragment shader.
    float radius_scalingfactor = length(PushConstants.modelview_matrix[0]);
	particle_radius_squared_fs = (radius * radius) * (radius_scalingfactor * radius_scalingfactor);
	particle_view_pos_fs = PushConstants.modelview_matrix * vec4(position, 1.0);

    // Calculate ray passing through the vertex (in view space).
    calculate_view_ray(vec2(gl_Position.x / gl_Position.w, gl_Position.y / gl_Position.w), ray_origin, ray_dir);
}
