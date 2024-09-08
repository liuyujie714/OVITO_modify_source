#version 440

#include "../global_uniforms.glsl"

// Push constants:
layout(push_constant) uniform constants {
    mat4 mvp;
    vec4 view_dir_eye_pos; // Either camera viewing direction (parallel) or camera position (perspective) in object space coordinates.
} PushConstants;

// Inputs:
layout(location = 0) in vec3 base;
layout(location = 1) in vec3 head;
layout(location = 2) in float radius;
layout(location = 3) in vec4 color1;
layout(location = 4) in vec4 color2;

// Outputs:
layout(location = 0) out vec4 color_fs;
out gl_PerVertex { vec4 gl_Position; };

void main()
{
	// Const array of vertex positions for the quad triangle strip.
	const vec2 quad[4] = vec2[4](
        vec2( 0.0, -1.0),
        vec2( 1.0, -1.0),
        vec2( 0.0,  1.0),
        vec2( 1.0,  1.0)
	);

    // The index of the quad corner.
    int corner = gl_VertexIndex;

    // Vector pointing from camera to cylinder base in object space:
	vec3 view_dir;
	if(!is_perspective())
		view_dir = PushConstants.view_dir_eye_pos.xyz;
	else
		view_dir = PushConstants.view_dir_eye_pos.xyz - base;

	// Build local coordinate system in object space.
    mat2x3 uv_tm;
	uv_tm[0] = head - base;
    uv_tm[1] = normalize(cross(view_dir, uv_tm[0])) * radius;

	// Project corner vertex.
    gl_Position = PushConstants.mvp * vec4(base + uv_tm * quad[corner], 1.0);

    // Forward primitive color to fragment shader.
    color_fs = (corner == 0 || corner == 2) ? color1 : color2;
}
