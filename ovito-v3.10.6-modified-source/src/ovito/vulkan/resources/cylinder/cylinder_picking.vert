#version 440

#include "../global_uniforms.glsl"
#include "../picking.glsl"

// Push constants:
layout(push_constant) uniform constants {
    mat4 mvp;
    layout(row_major) mat4x3 modelview_matrix;
    int pickingBaseId;
} PushConstants;

// Inputs:
layout(location = 0) in vec3 base;
layout(location = 1) in vec3 head;
layout(location = 2) in float radius;

// Outputs:
layout(location = 0) out vec4 color_fs;
layout(location = 1) out vec3 cylinder_view_base;		// Transformed cylinder position in view coordinates
layout(location = 2) out vec3 cylinder_view_axis;		// Transformed cylinder axis in view coordinates
layout(location = 3) out float cylinder_radius_sq_fs;	// The squared radius of the cylinder
layout(location = 4) out float cylinder_length;			// The length of the cylinder
layout(location = 5) out vec3 ray_origin;
layout(location = 6) out vec3 ray_dir;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
	// Const array of vertex positions for the box triangle strip.
	const vec3 box[14] = vec3[14](
        vec3( 1.0,  1.0,  1.0),
        vec3( 1.0, -1.0,  1.0),
        vec3( 1.0,  1.0,  0.0),
        vec3( 1.0, -1.0,  0.0),
        vec3(-1.0, -1.0,  0.0),
        vec3( 1.0, -1.0,  1.0),
        vec3(-1.0, -1.0,  1.0),
        vec3( 1.0,  1.0,  1.0),
        vec3(-1.0,  1.0,  1.0),
        vec3( 1.0,  1.0,  0.0),
        vec3(-1.0,  1.0,  0.0),
        vec3(-1.0, -1.0,  0.0),
        vec3(-1.0,  1.0,  1.0),
        vec3(-1.0, -1.0,  1.0)
	);

    // The index of the box corner.
    int corner = gl_VertexIndex;

    // Set up an axis tripod that is aligned with the cylinder.
    mat3 orientation_tm;
    orientation_tm[2] = head - base;
    if(orientation_tm[2] != vec3(0.0)) {
        if(orientation_tm[2].y != 0.0 || orientation_tm[2].x != 0.0)
            orientation_tm[0] = normalize(vec3(orientation_tm[2].y, -orientation_tm[2].x, 0.0)) * radius;
        else
            orientation_tm[0] = normalize(vec3(-orientation_tm[2].z, 0.0, orientation_tm[2].x)) * radius;
        orientation_tm[1] = normalize(cross(orientation_tm[2], orientation_tm[0])) * radius;
    }
    else {
        orientation_tm = mat3(0.0);
    }

	// Apply model-view-projection matrix to box vertex position.
    gl_Position = PushConstants.mvp * vec4(base + (orientation_tm * box[corner]), 1.0);

    // Compute color from object ID.
    color_fs = pickingModeColor(PushConstants.pickingBaseId, gl_InstanceIndex);

    // Apply additional scaling to cylinder radius due to model-view transformation. 
    float viewspace_radius = radius * length(PushConstants.modelview_matrix[0]);

	// Pass square of cylinder radius to fragment shader.
	cylinder_radius_sq_fs = viewspace_radius * viewspace_radius;

	// Transform cylinder to eye coordinates.
	cylinder_view_base = PushConstants.modelview_matrix * vec4(base, 1.0);
	cylinder_view_axis = PushConstants.modelview_matrix * vec4(orientation_tm[2], 0.0);

	// Pass cylinder length to fragment shader.
	cylinder_length = length(cylinder_view_axis);

    // Calculate ray passing through the vertex (in view space).
    calculate_view_ray(vec2(gl_Position.x / gl_Position.w, gl_Position.y / gl_Position.w), ray_origin, ray_dir);
}
