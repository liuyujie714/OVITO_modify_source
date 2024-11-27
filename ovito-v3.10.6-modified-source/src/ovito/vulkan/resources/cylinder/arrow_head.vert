#version 440

#include "../global_uniforms.glsl"

// Push constants:
layout(push_constant) uniform constants {
    mat4 mvp;
    layout(row_major) mat4x3 modelview_matrix;
} PushConstants;

// Inputs:
layout(location = 0) in vec3 base;
layout(location = 1) in vec3 head;
layout(location = 2) in float radius;
layout(location = 3) in vec4 color1;
layout(location = 4) in vec4 color2;

// Outputs:
layout(location = 0) out vec4 color_fs;
layout(location = 1) out vec3 center;	// Transformed cone vertex in view coordinates
layout(location = 2) out vec3 axis;		// Transformed cone axis in view coordinates
layout(location = 3) out float cone_radius;	// The radius of the cone
layout(location = 4) out vec3 ray_origin;
layout(location = 5) out vec3 ray_dir;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
	// Const array of vertex positions for the box triangle strip.
	const vec3 box[14] = vec3[14](
        vec3( 1.0,  1.0,  0.0),
        vec3( 1.0, -1.0,  0.0),
        vec3( 1.0,  1.0, -1.0),
        vec3( 1.0, -1.0, -1.0),
        vec3(-1.0, -1.0, -1.0),
        vec3( 1.0, -1.0,  0.0),
        vec3(-1.0, -1.0,  0.0),
        vec3( 1.0,  1.0,  0.0),
        vec3(-1.0,  1.0,  0.0),
        vec3( 1.0,  1.0, -1.0),
        vec3(-1.0,  1.0, -1.0),
        vec3(-1.0, -1.0, -1.0),
        vec3(-1.0,  1.0,  0.0),
        vec3(-1.0, -1.0,  0.0)
	);

    // The index of the box corner.
    int corner = gl_VertexIndex;

    float arrowHeadRadius = radius * 2.5;
    float arrowHeadLength = (arrowHeadRadius * 1.8);

    // Set up an axis tripod that is aligned with the cone.
    mat3 orientation_tm;
    orientation_tm[2] = head - base;
    float len = length(orientation_tm[2]);
    if(len != 0.0) {
        if(arrowHeadLength > len) {
            arrowHeadRadius *= len / arrowHeadLength;
            arrowHeadLength = len;
        }
        orientation_tm[2] *= arrowHeadLength / len;

        if(orientation_tm[2].y != 0.0 || orientation_tm[2].x != 0.0)
            orientation_tm[0] = normalize(vec3(orientation_tm[2].y, -orientation_tm[2].x, 0.0)) * arrowHeadRadius;
        else
            orientation_tm[0] = normalize(vec3(-orientation_tm[2].z, 0.0, orientation_tm[2].x)) * arrowHeadRadius;
        orientation_tm[1] = normalize(cross(orientation_tm[2], orientation_tm[0])) * arrowHeadRadius;
    }
    else {
        orientation_tm = mat3(0.0);
    }

	// Apply model-view-projection matrix to box vertex position.
    gl_Position = PushConstants.mvp * vec4(head + (orientation_tm * box[corner]), 1.0);

    // Forward cylinder color to fragment shader.
    color_fs = color1;

    // Apply additional scaling to cone radius due to model-view transformation. 
	// Pass square of cylinder radius to fragment shader.
    cone_radius = arrowHeadRadius * length(PushConstants.modelview_matrix[0]);

	// Transform cone to eye coordinates.
    center = PushConstants.modelview_matrix * vec4(head, 1.0);
    axis = PushConstants.modelview_matrix * vec4(-orientation_tm[2], 0.0);

    // Calculate ray passing through the vertex (in view space).
    calculate_view_ray(vec2(gl_Position.x / gl_Position.w, gl_Position.y / gl_Position.w), ray_origin, ray_dir);
}
