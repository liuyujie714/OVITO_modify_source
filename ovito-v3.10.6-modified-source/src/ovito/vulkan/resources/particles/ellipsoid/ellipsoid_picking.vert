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
layout(location = 3) in mat4 shape_orientation;

// Outputs:
layout(location = 0) out vec4 color_fs;
layout(location = 1) out mat3 view_to_sphere_fs;
layout(location = 4) out vec3 particle_view_pos_fs;
layout(location = 5) out vec3 ray_origin;
layout(location = 6) out vec3 ray_dir;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
	// Const array of vertex positions for the unit cube triangle strip.
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

    // The index of the box corner.
    int corner = gl_VertexIndex;

    // Compute rotated and scaled unit cube corner coordinates.
    vec4 scaled_corner = vec4(position, 1.0) + (shape_orientation * vec4(cube[corner], 0.0));

	// Apply model-view-projection matrix to particle position displaced by the cube vertex position.
    gl_Position = PushConstants.mvp * scaled_corner;

    // Compute color from object ID.
    color_fs = pickingModeColor(PushConstants.pickingBaseId, gl_InstanceIndex);

    // Pass ellipsoid matrix and center position to fragment shader.
	particle_view_pos_fs = PushConstants.modelview_matrix * vec4(position, 1.0);
    view_to_sphere_fs = inverse(mat3(PushConstants.modelview_matrix) * mat3(shape_orientation));

    // Calculate ray passing through the vertex (in view space).
    calculate_view_ray(vec2(gl_Position.x / gl_Position.w, gl_Position.y / gl_Position.w), ray_origin, ray_dir);
}
